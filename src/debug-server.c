/*
 * starter-core - debug-server.c
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gio/gio.h>

#include <starter/error.h>
#include <starter/client.h>

#include "debug-server.h"

#define ST_CORE_DEBUG_OBJECT_PATH "/org/starter/core/debug"
#define ST_CORE_DEBUG_INTERFACE "org.starter.Core.Debug"

typedef struct
{
  gchar *address;
  guint16 port;
  guint state;
} DebugPeer;

typedef struct
{
  guint handle;
  gchar *id;
} DebugPoint;

typedef struct
{
  STCore *core;			/* not owned */
  GDBusServer *server;
  GPtrArray *connections;
  GDBusNodeInfo *introspection_data;
  GHashTable *peers;		/* name -> DebugPeer */
  GMainContext *listen_context;	/* context where GDBusServer was started */
  gchar *bind_host;
  guint16 bind_port;
  gchar *client_address;

  GHashTable *breakpoints;	/* handle(guint) -> DebugPoint */
  GHashTable *watchpoints;	/* handle(guint) -> DebugPoint */
  guint next_handle;
  gboolean paused;
  gchar *stop_reason;		/* "breakpoint" | "watchpoint" | NULL */
  gchar *stop_id;
  gdouble stop_value;
  guint stop_handle;
} STCoreDebugPrivate;

struct _STCoreDebug
{
  GObject parent_instance;
};

G_DEFINE_TYPE_WITH_PRIVATE (STCoreDebug, st_core_debug, G_TYPE_OBJECT)

#define ST_CORE_DEBUG_GET_PRIVATE(obj) \
  ((STCoreDebugPrivate *) st_core_debug_get_instance_private (ST_CORE_DEBUG (obj)))

static const gchar introspection_xml[] =
  "<node name='/org/starter/core/debug'>"
  "  <interface name='org.starter.Core.Debug'>"
  "    <method name='GetStatus'>"
  "      <arg direction='out' type='a{sv}' name='status'/>"
  "    </method>"
  "    <method name='ListPeers'>"
  "      <arg direction='out' type='a(ssuu)' name='peers'/>"
  "    </method>"
  "    <method name='ListInputs'>"
  "      <arg direction='out' type='a{s(siddddt)}' name='inputs'/>"
  "    </method>"
  "    <method name='ListOutputs'>"
  "      <arg direction='out' type='a{s(siddddt)}' name='outputs'/>"
  "    </method>"
  "    <method name='SetInput'>"
  "      <arg direction='in' type='s' name='id'/>"
  "      <arg direction='in' type='d' name='value'/>"
  "      <arg direction='in' type='t' name='flags'/>"
  "      <arg direction='out' type='i' name='ok'/>"
  "    </method>"
  "    <method name='SetOutput'>"
  "      <arg direction='in' type='s' name='id'/>"
  "      <arg direction='in' type='d' name='value'/>"
  "      <arg direction='in' type='t' name='flags'/>"
  "      <arg direction='out' type='i' name='ok'/>"
  "    </method>"
  "    <method name='AddBreakpoint'>"
  "      <arg direction='in' type='s' name='id'/>"
  "      <arg direction='out' type='u' name='handle'/>"
  "    </method>"
  "    <method name='RemoveBreakpoint'>"
  "      <arg direction='in' type='u' name='handle'/>"
  "      <arg direction='out' type='b' name='ok'/>"
  "    </method>"
  "    <method name='ListBreakpoints'>"
  "      <arg direction='out' type='a(us)' name='breakpoints'/>"
  "    </method>"
  "    <method name='AddWatchpoint'>"
  "      <arg direction='in' type='s' name='id'/>"
  "      <arg direction='out' type='u' name='handle'/>"
  "    </method>"
  "    <method name='RemoveWatchpoint'>"
  "      <arg direction='in' type='u' name='handle'/>"
  "      <arg direction='out' type='b' name='ok'/>"
  "    </method>"
  "    <method name='ListWatchpoints'>"
  "      <arg direction='out' type='a(us)' name='watchpoints'/>"
  "    </method>"
  "    <method name='Continue'>"
  "    </method>"
  "    <method name='GetPaused'>"
  "      <arg direction='out' type='b' name='paused'/>"
  "    </method>"
  "    <signal name='PeerChanged'>"
  "      <arg name='name' type='s'/>"
  "      <arg name='address' type='s'/>"
  "      <arg name='port' type='u'/>"
  "      <arg name='state' type='u'/>"
  "      <arg name='event' type='s'/>"
  "    </signal>"
  "    <signal name='InputChanged'>"
  "      <arg name='object' type='{s(siddddt)}'/>"
  "    </signal>"
  "    <signal name='OutputChanged'>"
  "      <arg name='object' type='{s(siddddt)}'/>"
  "    </signal>"
  "    <signal name='StatsChanged'>"
  "      <arg name='name' type='s'/>"
  "    </signal>"
  "    <signal name='VarChanged'>"
  "      <arg name='name' type='s'/>"
  "      <arg name='describe' type='s'/>"
  "    </signal>"
  "    <signal name='RuleTriggered'>"
  "      <arg name='dest' type='s'/>"
  "      <arg name='describe' type='s'/>"
  "    </signal>"
  "    <signal name='TriggerTriggered'>"
  "      <arg name='dest' type='s'/>"
  "      <arg name='describe' type='s'/>"
  "    </signal>"
  "    <signal name='Stopped'>"
  "      <arg name='reason' type='s'/>"
  "      <arg name='id' type='s'/>"
  "      <arg name='value' type='d'/>"
  "      <arg name='handle' type='u'/>"
  "    </signal>"
  "  </interface>"
  "</node>";

static void
debug_peer_free (gpointer data)
{
  DebugPeer *peer = data;

  if (peer == NULL)
    return;
  g_free (peer->address);
  g_free (peer);
}

static void
debug_point_free (gpointer data)
{
  DebugPoint *point = data;

  if (point == NULL)
    return;
  g_free (point->id);
  g_free (point);
}

static gboolean
parse_bind_spec (const gchar * spec, gchar ** host_out, guint16 * port_out,
		 GError ** error)
{
  const gchar *colon;
  gchar *host;
  unsigned long port;
  char *end = NULL;

  g_return_val_if_fail (spec != NULL, FALSE);
  g_return_val_if_fail (host_out != NULL, FALSE);
  g_return_val_if_fail (port_out != NULL, FALSE);

  colon = strrchr (spec, ':');
  if (colon == NULL || colon == spec || *(colon + 1) == '\0')
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
		   "invalid debug-server bind '%s' (expected HOST:PORT)",
		   spec);
      return FALSE;
    }

  port = strtoul (colon + 1, &end, 10);
  if (end == colon + 1 || *end != '\0' || port > 65535)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
		   "invalid debug-server port in '%s'", spec);
      return FALSE;
    }

  host = g_strndup (spec, (gsize) (colon - spec));
  if (*host == '\0')
    {
      g_free (host);
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
		   "empty host in debug-server bind '%s'", spec);
      return FALSE;
    }

  *host_out = host;
  *port_out = (guint16) port;
  return TRUE;
}

/*
 * Emit on the listen-thread context. Production uses a single GMainLoop, so
 * callers already own that context. Tests that drive the server on a worker
 * thread must marshal emit_* via g_main_context_invoke on listen_context.
 */
static void
broadcast_signal (STCoreDebug * self, const gchar * signal_name,
		  GVariant * parameters)
{
  STCoreDebugPrivate *priv;
  guint i;

  if (self == NULL || !ST_IS_CORE_DEBUG (self) || parameters == NULL)
    {
      if (parameters)
	g_variant_unref (g_variant_ref_sink (parameters));
      return;
    }

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  parameters = g_variant_ref_sink (parameters);

  if (priv->listen_context
      && !g_main_context_is_owner (priv->listen_context))
    {
      /* Snapshot updates still apply; D-Bus emit requires the listen thread. */
      LOGD ("debug-server: signal %s emitted off listen thread; dropped",
	    signal_name);
      g_variant_unref (parameters);
      return;
    }

  for (i = 0; i < priv->connections->len; i++)
    {
      GDBusConnection *conn = priv->connections->pdata[i];

      g_dbus_connection_emit_signal (conn, NULL, ST_CORE_DEBUG_OBJECT_PATH,
				     ST_CORE_DEBUG_INTERFACE, signal_name,
				     g_variant_ref (parameters), NULL);
    }

  g_variant_unref (parameters);
}

static void
peers_upsert (STCoreDebugPrivate * priv, const gchar * name,
	      const gchar * address, guint16 port, guint state)
{
  DebugPeer *peer;

  peer = g_hash_table_lookup (priv->peers, name);
  if (peer == NULL)
    {
      peer = g_new0 (DebugPeer, 1);
      g_hash_table_insert (priv->peers, g_strdup (name), peer);
    }

  g_free (peer->address);
  peer->address = g_strdup (address ? address : "");
  peer->port = port;
  peer->state = state;
}

static void
handle_get_status (STCoreDebug * self, GDBusMethodInvocation * invocation)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  STCore *core = priv->core;
  GVariantBuilder builder;
  guint n_peers = 0;
  guint n_pending = 0;
  guint n_inputs = 0;
  guint n_outputs = 0;

  if (priv->peers)
    n_peers = g_hash_table_size (priv->peers);
  if (core->pending_clients)
    n_pending = g_hash_table_size (core->pending_clients);
  if (core->all_inputs)
    n_inputs = g_hash_table_size (core->all_inputs);
  if (core->all_outputs)
    n_outputs = g_hash_table_size (core->all_outputs);

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));
  g_variant_builder_add (&builder, "{sv}", "package",
			 g_variant_new_string (PACKAGE));
  g_variant_builder_add (&builder, "{sv}", "version",
			 g_variant_new_string (VERSION));
  g_variant_builder_add (&builder, "{sv}", "rules-file",
			 g_variant_new_string (core->rules_file ? core->
					       rules_file : ""));
  g_variant_builder_add (&builder, "{sv}", "service-match",
			 g_variant_new_string (core->srvmatch ? core->
					       srvmatch : ""));
  g_variant_builder_add (&builder, "{sv}", "bind-host",
			 g_variant_new_string (priv->bind_host ? priv->
					       bind_host : ""));
  g_variant_builder_add (&builder, "{sv}", "bind-port",
			 g_variant_new_uint16 (priv->bind_port));
  g_variant_builder_add (&builder, "{sv}", "client-address",
			 g_variant_new_string (priv->client_address ? priv->
					       client_address : ""));
  g_variant_builder_add (&builder, "{sv}", "peers",
			 g_variant_new_uint32 (n_peers));
  g_variant_builder_add (&builder, "{sv}", "pending-peers",
			 g_variant_new_uint32 (n_pending));
  g_variant_builder_add (&builder, "{sv}", "inputs",
			 g_variant_new_uint32 (n_inputs));
  g_variant_builder_add (&builder, "{sv}", "outputs",
			 g_variant_new_uint32 (n_outputs));
  g_variant_builder_add (&builder, "{sv}", "no-discovery",
			 g_variant_new_boolean (core->no_discovery));
  g_variant_builder_add (&builder, "{sv}", "paused",
			 g_variant_new_boolean (priv->paused));
  g_variant_builder_add (&builder, "{sv}", "stop-reason",
			 g_variant_new_string (priv->stop_reason ? priv->
					       stop_reason : ""));
  g_variant_builder_add (&builder, "{sv}", "stop-id",
			 g_variant_new_string (priv->stop_id ? priv->
					       stop_id : ""));
  g_variant_builder_add (&builder, "{sv}", "breakpoints",
			 g_variant_new_uint32 (g_hash_table_size
					       (priv->breakpoints)));
  g_variant_builder_add (&builder, "{sv}", "watchpoints",
			 g_variant_new_uint32 (g_hash_table_size
					       (priv->watchpoints)));

  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(a{sv})", &builder));
}

static void
handle_list_peers (STCoreDebug * self, GDBusMethodInvocation * invocation)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  GVariantBuilder builder;
  GHashTableIter iter;
  gpointer key, value;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(ssuu)"));

  g_hash_table_iter_init (&iter, priv->peers);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      const gchar *name = key;
      DebugPeer *peer = value;

      g_variant_builder_add (&builder, "(ssuu)", name,
			     peer->address ? peer->address : "",
			     (guint) peer->port, peer->state);
    }

  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(a(ssuu))",
							&builder));
}

static STInput *lookup_input (STCore * core, const gchar * id);
static STOutput *lookup_output (STCore * core, const gchar * id);

static void
handle_list_inputs (STCoreDebug * self, GDBusMethodInvocation * invocation)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  STCore *core = priv->core;
  GVariantBuilder builder;
  GHashTableIter iter;
  gpointer key, value;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{s(siddddt)}"));
  if (core != NULL && core->all_inputs != NULL)
    {
      g_hash_table_iter_init (&iter, core->all_inputs);
      while (g_hash_table_iter_next (&iter, &key, &value))
	{
	  const gchar *id = key;
	  STInput *in = lookup_input (core, id);

	  if (in == NULL)
	    continue;
	  /* Dict key is the all_inputs id (same as SetInput / signals). */
	  g_variant_builder_add (&builder, "{s(siddddt)}",
				 id,
				 st_input_get_description (in),
				 st_input_get_unit (in),
				 st_input_get_min (in),
				 st_input_get_max (in),
				 st_input_get_step (in),
				 st_input_get_val (in),
				 st_input_get_flags (in));
	}
    }

  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(a{s(siddddt)})",
							&builder));
}

static void
handle_list_outputs (STCoreDebug * self, GDBusMethodInvocation * invocation)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  STCore *core = priv->core;
  GVariantBuilder builder;
  GHashTableIter iter;
  gpointer key, value;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{s(siddddt)}"));
  if (core != NULL && core->all_outputs != NULL)
    {
      g_hash_table_iter_init (&iter, core->all_outputs);
      while (g_hash_table_iter_next (&iter, &key, &value))
	{
	  const gchar *id = key;
	  STOutput *out = lookup_output (core, id);

	  if (out == NULL)
	    continue;
	  g_variant_builder_add (&builder, "{s(siddddt)}",
				 id,
				 st_output_get_description (out),
				 st_output_get_unit (out),
				 st_output_get_min (out),
				 st_output_get_max (out),
				 st_output_get_step (out),
				 st_output_get_val (out),
				 st_output_get_flags (out));
	}
    }

  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(a{s(siddddt)})",
							&builder));
}

static void
handle_set_input (STCoreDebug * self, GVariant * parameters,
		  GDBusMethodInvocation * invocation)
{
  const gchar *id = NULL;
  gdouble value = 0;
  guint64 flags = 0;
  GError *error = NULL;
  gboolean ok;

  g_variant_get (parameters, "(&sdt)", &id, &value, &flags);
  ok = st_core_debug_set_input (self, id, value, flags, &error);
  if (!ok)
    LOGD ("debug-server SetInput %s failed: %s", id,
	  error ? error->message : "unknown");
  g_clear_error (&error);

  /* Match org.starter.Service SetOutput style: 1 = success, 0 = failure. */
  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(i)", ok ? 1 : 0));
}

static void
handle_set_output (STCoreDebug * self, GVariant * parameters,
		   GDBusMethodInvocation * invocation)
{
  const gchar *id = NULL;
  gdouble value = 0;
  guint64 flags = 0;
  GError *error = NULL;
  gboolean ok;

  g_variant_get (parameters, "(&sdt)", &id, &value, &flags);
  ok = st_core_debug_set_output (self, id, value, flags, &error);
  if (!ok)
    LOGD ("debug-server SetOutput %s failed: %s", id,
	  error ? error->message : "unknown");
  g_clear_error (&error);

  g_dbus_method_invocation_return_value (invocation,
					 g_variant_new ("(i)", ok ? 1 : 0));
}

static void
handle_method_call (GDBusConnection * connection,
		    const gchar * sender,
		    const gchar * object_path,
		    const gchar * interface_name,
		    const gchar * method_name,
		    GVariant * parameters,
		    GDBusMethodInvocation * invocation, gpointer user_data)
{
  STCoreDebug *self = user_data;

  (void) connection;
  (void) sender;
  (void) object_path;
  (void) interface_name;

  LOGD ("debug-server method %s", method_name);

  if (g_strcmp0 (method_name, "GetStatus") == 0)
    handle_get_status (self, invocation);
  else if (g_strcmp0 (method_name, "ListPeers") == 0)
    handle_list_peers (self, invocation);
  else if (g_strcmp0 (method_name, "ListInputs") == 0)
    handle_list_inputs (self, invocation);
  else if (g_strcmp0 (method_name, "ListOutputs") == 0)
    handle_list_outputs (self, invocation);
  else if (g_strcmp0 (method_name, "SetInput") == 0)
    handle_set_input (self, parameters, invocation);
  else if (g_strcmp0 (method_name, "SetOutput") == 0)
    handle_set_output (self, parameters, invocation);
  else if (g_strcmp0 (method_name, "AddBreakpoint") == 0)
    {
      const gchar *id = NULL;
      GError *error = NULL;
      guint handle;

      g_variant_get (parameters, "(&s)", &id);
      handle = st_core_debug_add_breakpoint (self, id, &error);
      g_clear_error (&error);
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(u)", handle));
    }
  else if (g_strcmp0 (method_name, "RemoveBreakpoint") == 0)
    {
      guint handle = 0;
      GError *error = NULL;
      gboolean ok;

      g_variant_get (parameters, "(u)", &handle);
      ok = st_core_debug_remove_breakpoint (self, handle, &error);
      g_clear_error (&error);
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(b)", ok));
    }
  else if (g_strcmp0 (method_name, "ListBreakpoints") == 0)
    {
      STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
      GVariantBuilder builder;
      GHashTableIter iter;
      gpointer key, value;

      g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(us)"));
      g_hash_table_iter_init (&iter, priv->breakpoints);
      while (g_hash_table_iter_next (&iter, &key, &value))
	{
	  DebugPoint *point = value;

	  g_variant_builder_add (&builder, "(us)", point->handle, point->id);
	}
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(a(us))",
							    &builder));
    }
  else if (g_strcmp0 (method_name, "AddWatchpoint") == 0)
    {
      const gchar *id = NULL;
      GError *error = NULL;
      guint handle;

      g_variant_get (parameters, "(&s)", &id);
      handle = st_core_debug_add_watchpoint (self, id, &error);
      g_clear_error (&error);
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(u)", handle));
    }
  else if (g_strcmp0 (method_name, "RemoveWatchpoint") == 0)
    {
      guint handle = 0;
      GError *error = NULL;
      gboolean ok;

      g_variant_get (parameters, "(u)", &handle);
      ok = st_core_debug_remove_watchpoint (self, handle, &error);
      g_clear_error (&error);
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(b)", ok));
    }
  else if (g_strcmp0 (method_name, "ListWatchpoints") == 0)
    {
      STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
      GVariantBuilder builder;
      GHashTableIter iter;
      gpointer key, value;

      g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(us)"));
      g_hash_table_iter_init (&iter, priv->watchpoints);
      while (g_hash_table_iter_next (&iter, &key, &value))
	{
	  DebugPoint *point = value;

	  g_variant_builder_add (&builder, "(us)", point->handle, point->id);
	}
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(a(us))",
							    &builder));
    }
  else if (g_strcmp0 (method_name, "Continue") == 0)
    {
      st_core_debug_continue (self);
      g_dbus_method_invocation_return_value (invocation, NULL);
    }
  else if (g_strcmp0 (method_name, "GetPaused") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(b)",
							    st_core_debug_is_paused
							    (self)));
    }
  else
    g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
					   G_DBUS_ERROR_UNKNOWN_METHOD,
					   "Unknown method %s", method_name);
}

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  NULL,
  NULL,
};

static void
connection_closed (GDBusConnection * connection,
		   gboolean remote_peer_vanished, GError * error,
		   gpointer user_data)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (user_data);

  (void) remote_peer_vanished;
  (void) error;

  LOGD ("debug-server client disconnected");
  g_ptr_array_remove (priv->connections, connection);
}

static gboolean
on_new_connection (GDBusServer * server, GDBusConnection * connection,
		   gpointer user_data)
{
  STCoreDebug *self = user_data;
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  guint registration_id;
  GError *error = NULL;

  (void) server;

  LOGI ("debug-server: client connected");

  g_object_ref (connection);
  g_ptr_array_add (priv->connections, connection);

  g_signal_connect (connection, "closed", G_CALLBACK (connection_closed),
		    self);

  registration_id =
    g_dbus_connection_register_object (connection,
				       ST_CORE_DEBUG_OBJECT_PATH,
				       priv->introspection_data->
				       interfaces[0], &interface_vtable, self,
				       NULL, &error);
  if (registration_id == 0)
    {
      LOGE ("debug-server: register_object failed: %s",
	    error ? error->message : "unknown");
      g_clear_error (&error);
      g_ptr_array_remove (priv->connections, connection);
      return FALSE;
    }

  return TRUE;
}

static void
st_core_debug_dispose (GObject * object)
{
  st_core_debug_stop (ST_CORE_DEBUG (object));
  G_OBJECT_CLASS (st_core_debug_parent_class)->dispose (object);
}

static void
st_core_debug_finalize (GObject * object)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (object);

  g_clear_pointer (&priv->connections, g_ptr_array_unref);
  g_clear_pointer (&priv->peers, g_hash_table_unref);
  g_clear_pointer (&priv->breakpoints, g_hash_table_unref);
  g_clear_pointer (&priv->watchpoints, g_hash_table_unref);
  g_clear_pointer (&priv->introspection_data, g_dbus_node_info_unref);
  g_clear_pointer (&priv->listen_context, g_main_context_unref);
  g_clear_pointer (&priv->bind_host, g_free);
  g_clear_pointer (&priv->client_address, g_free);
  g_clear_pointer (&priv->stop_reason, g_free);
  g_clear_pointer (&priv->stop_id, g_free);
  priv->core = NULL;

  G_OBJECT_CLASS (st_core_debug_parent_class)->finalize (object);
}

static void
st_core_debug_class_init (STCoreDebugClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = st_core_debug_dispose;
  gobject_class->finalize = st_core_debug_finalize;
}

static void
st_core_debug_init (STCoreDebug * self)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  priv->core = NULL;
  priv->server = NULL;
  priv->connections = g_ptr_array_new_full (0, g_object_unref);
  priv->peers =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, debug_peer_free);
  priv->breakpoints =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
			   debug_point_free);
  priv->watchpoints =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
			   debug_point_free);
  priv->next_handle = 1;
  priv->paused = FALSE;
  priv->stop_reason = NULL;
  priv->stop_id = NULL;
  priv->stop_value = 0;
  priv->stop_handle = 0;
  priv->introspection_data =
    g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  priv->listen_context = NULL;
  priv->bind_host = NULL;
  priv->bind_port = 0;
  priv->client_address = NULL;
}

STCoreDebug *
st_core_debug_new (STCore * core)
{
  STCoreDebug *self;
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (core != NULL, NULL);

  self = g_object_new (ST_TYPE_CORE_DEBUG, NULL);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  priv->core = core;
  return self;
}

gboolean
st_core_debug_start (STCoreDebug * self, const gchar * bind_spec,
		     GError ** error)
{
  STCoreDebugPrivate *priv;
  gchar *host = NULL;
  guint16 port = 0;
  gchar *guid = NULL;
  gchar *addr = NULL;
  const gchar *spec;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  if (priv->server != NULL)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_EXISTS,
		   "debug server already started");
      return FALSE;
    }

  spec = (bind_spec && *bind_spec) ? bind_spec : ST_CORE_DEBUG_DEFAULT_BIND;

  if (!parse_bind_spec (spec, &host, &port, error))
    return FALSE;

  guid = g_dbus_generate_guid ();
  addr = g_strdup_printf ("tcp:host=%s,port=%hu", host, port);

  LOGI ("debug-server: listening on %s", addr);
  LOGD ("debug-server: create GDBusServer at '%s'", addr);

  priv->server =
    g_dbus_server_new_sync (addr,
			    G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS,
			    guid, NULL, NULL, error);
  g_free (guid);
  g_free (addr);

  if (priv->server == NULL)
    {
      g_free (host);
      return FALSE;
    }

  {
    const gchar *client_addr = g_dbus_server_get_client_address (priv->server);
    const gchar *port_key;

    g_clear_pointer (&priv->bind_host, g_free);
    priv->bind_host = host;
    priv->bind_port = port;

    g_clear_pointer (&priv->client_address, g_free);
    if (client_addr)
      {
	priv->client_address = g_strdup (client_addr);
	port_key = strstr (client_addr, "port=");
	if (port_key != NULL)
	  {
	    unsigned int parsed = 0;

	    if (sscanf (port_key + 5, "%u", &parsed) == 1 && parsed <= 65535)
	      priv->bind_port = (guint16) parsed;
	  }
      }
  }

  g_clear_pointer (&priv->listen_context, g_main_context_unref);
  priv->listen_context = g_main_context_ref_thread_default ();

  g_signal_connect (priv->server, "new-connection",
		    G_CALLBACK (on_new_connection), self);
  g_dbus_server_start (priv->server);

  LOGI ("debug-server: ready (%s)",
	priv->client_address ? priv->client_address : "unknown address");

  return TRUE;
}

void
st_core_debug_stop (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  if (self == NULL || !ST_IS_CORE_DEBUG (self))
    return;

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  if (priv->connections)
    {
      guint i;

      for (i = 0; i < priv->connections->len; i++)
	g_signal_handlers_disconnect_by_data (priv->connections->pdata[i],
					      self);
      g_ptr_array_set_size (priv->connections, 0);
    }

  if (priv->server)
    {
      g_dbus_server_stop (priv->server);
      g_clear_object (&priv->server);
      LOGI ("debug-server: stopped");
    }

  g_clear_pointer (&priv->listen_context, g_main_context_unref);
}

gboolean
st_core_debug_is_active (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return priv->server != NULL;
}

const gchar *
st_core_debug_get_bind_host (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), NULL);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return priv->bind_host;
}

guint16
st_core_debug_get_bind_port (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), 0);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return priv->bind_port;
}

const gchar *
st_core_debug_get_client_address (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), NULL);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return priv->client_address;
}

void
st_core_debug_emit_peer_changed (STCoreDebug * self, const gchar * name,
				 const gchar * address, guint16 port,
				 guint state, const gchar * event)
{
  STCoreDebugPrivate *priv;
  g_autofree gchar *addr_owned = NULL;
  const gchar *addr;
  const gchar *ev;
  guint16 emit_port = port;

  if (self == NULL || !ST_IS_CORE_DEBUG (self) || name == NULL)
    return;

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  addr = address ? address : "";
  ev = event ? event : "";

  if (g_strcmp0 (ev, "removed") == 0 || g_strcmp0 (ev, "failed") == 0)
    {
      DebugPeer *existing = g_hash_table_lookup (priv->peers, name);

      if (existing != NULL && (*addr == '\0'))
	{
	  addr_owned = g_strdup (existing->address ? existing->address : "");
	  addr = addr_owned;
	  emit_port = existing->port;
	}
      g_hash_table_remove (priv->peers, name);
    }
  else
    peers_upsert (priv, name, addr, emit_port, state);

  LOGD ("debug-server PeerChanged %s %s:%u state=%u event=%s", name, addr,
	emit_port, state, ev);

  broadcast_signal (self, "PeerChanged",
		    g_variant_new ("(ssuus)", name, addr, (guint) emit_port,
				   state, ev));
}

void
st_core_debug_emit_input_changed (STCoreDebug * self, STInput * in)
{
  GVariant *item;

  if (self == NULL || !ST_IS_CORE_DEBUG (self) || in == NULL)
    return;

  item = g_variant_new ("{s(siddddt)}",
			st_input_get_name (in),
			st_input_get_description (in),
			st_input_get_unit (in),
			st_input_get_min (in),
			st_input_get_max (in),
			st_input_get_step (in),
			st_input_get_val (in), st_input_get_flags (in));

  broadcast_signal (self, "InputChanged", g_variant_new_tuple (&item, 1));
}

void
st_core_debug_emit_output_changed (STCoreDebug * self, STOutput * out)
{
  GVariant *item;

  if (self == NULL || !ST_IS_CORE_DEBUG (self) || out == NULL)
    return;

  item = g_variant_new ("{s(siddddt)}",
			st_output_get_name (out),
			st_output_get_description (out),
			st_output_get_unit (out),
			st_output_get_min (out),
			st_output_get_max (out),
			st_output_get_step (out),
			st_output_get_val (out), st_output_get_flags (out));

  broadcast_signal (self, "OutputChanged", g_variant_new_tuple (&item, 1));
}

void
st_core_debug_emit_stats_changed (STCoreDebug * self, STStats * stats)
{
  if (self == NULL || !ST_IS_CORE_DEBUG (self) || stats == NULL)
    return;

  broadcast_signal (self, "StatsChanged",
		    g_variant_new ("(s)", st_stats_get_name (stats)));
}

void
st_core_debug_emit_var_changed (STCoreDebug * self, STVar * var)
{
  g_autoptr (GString) s = NULL;
  const gchar *desc;

  if (self == NULL || !ST_IS_CORE_DEBUG (self) || var == NULL)
    return;

  s = g_string_new ("");
  desc = expr_describe (st_var_get_value (var), s);
  broadcast_signal (self, "VarChanged",
		    g_variant_new ("(ss)", st_var_get_name (var),
				   desc ? desc : ""));
}

void
st_core_debug_emit_rule_triggered (STCoreDebug * self, const gchar * dest,
				   const gchar * describe)
{
  if (self == NULL || !ST_IS_CORE_DEBUG (self) || dest == NULL)
    return;

  broadcast_signal (self, "RuleTriggered",
		    g_variant_new ("(ss)", dest,
				   describe ? describe : ""));
}

void
st_core_debug_emit_trigger_triggered (STCoreDebug * self, const gchar * dest,
				      const gchar * describe)
{
  if (self == NULL || !ST_IS_CORE_DEBUG (self) || dest == NULL)
    return;

  broadcast_signal (self, "TriggerTriggered",
		    g_variant_new ("(ss)", dest,
				   describe ? describe : ""));
}

static STInput *
lookup_input (STCore * core, const gchar * id)
{
  ClientInputMap *map;
  const GPtrArray *inputs;

  if (core == NULL || core->all_inputs == NULL || id == NULL)
    return NULL;

  map = g_hash_table_lookup (core->all_inputs, id);
  if (map == NULL || map->client == NULL)
    return NULL;

  inputs = st_client_get_inputs (map->client);
  if (inputs == NULL || map->index >= inputs->len)
    return NULL;

  return inputs->pdata[map->index];
}

static STOutput *
lookup_output (STCore * core, const gchar * id)
{
  ClientOutputMap *map;
  const GPtrArray *outputs;

  if (core == NULL || core->all_outputs == NULL || id == NULL)
    return NULL;

  map = g_hash_table_lookup (core->all_outputs, id);
  if (map == NULL || map->client == NULL)
    return NULL;

  outputs = st_client_get_outputs (map->client);
  if (outputs == NULL || map->index >= outputs->len)
    return NULL;

  return outputs->pdata[map->index];
}

gboolean
st_core_debug_set_input (STCoreDebug * self, const gchar * id, gdouble value,
			 guint64 flags, GError ** error)
{
  STCoreDebugPrivate *priv;
  STInput *in;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  g_return_val_if_fail (id != NULL, FALSE);

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  in = lookup_input (priv->core, id);
  if (in == NULL)
    {
      g_set_error (error, ST_ERROR, ST_ERROR_UNDEFINED_IDENTIFIER,
		   "unknown input '%s'", id);
      return FALSE;
    }

  if (!st_input_set_val (in, value, error))
    return FALSE;
  if (!st_input_set_flags (in, flags, error))
    return FALSE;

  LOGI ("debug-server SetInput %s = %g flags=%" G_GUINT64_FORMAT, id, value,
	flags);
  return TRUE;
}

gboolean
st_core_debug_set_output (STCoreDebug * self, const gchar * id, gdouble value,
			  guint64 flags, GError ** error)
{
  STCoreDebugPrivate *priv;
  ClientOutputMap *map;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  g_return_val_if_fail (id != NULL, FALSE);

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  if (priv->core == NULL || priv->core->all_outputs == NULL)
    {
      g_set_error (error, ST_ERROR, ST_ERROR_UNDEFINED_IDENTIFIER,
		   "unknown output '%s'", id);
      return FALSE;
    }

  map = g_hash_table_lookup (priv->core->all_outputs, id);
  if (map == NULL || map->client == NULL)
    {
      g_set_error (error, ST_ERROR, ST_ERROR_UNDEFINED_IDENTIFIER,
		   "unknown output '%s'", id);
      return FALSE;
    }

  /* Debugger force: always call through, ignore MANUAL_OVERRIDE gating. */
  if (!st_client_set_output (map->client, id, value, flags))
    {
      g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE,
		   "SetOutput failed for '%s'", id);
      return FALSE;
    }

  LOGI ("debug-server SetOutput %s = %g flags=%" G_GUINT64_FORMAT, id, value,
	flags);
  return TRUE;
}

static guint
add_point (GHashTable * table, guint * next_handle, const gchar * id,
	   GError ** error)
{
  DebugPoint *point;
  guint handle;

  if (id == NULL || *id == '\0')
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
		   "empty breakpoint/watchpoint id");
      return 0;
    }

  handle = (*next_handle)++;
  if (handle == 0)
    handle = (*next_handle)++;

  point = g_new0 (DebugPoint, 1);
  point->handle = handle;
  point->id = g_strdup (id);
  g_hash_table_insert (table, GUINT_TO_POINTER (handle), point);
  return handle;
}

static gboolean
remove_point (GHashTable * table, guint handle, GError ** error)
{
  if (handle == 0 || !g_hash_table_remove (table, GUINT_TO_POINTER (handle)))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
		   "unknown handle %u", handle);
      return FALSE;
    }
  return TRUE;
}

static DebugPoint *
find_point_by_id (GHashTable * table, const gchar * id)
{
  GHashTableIter iter;
  gpointer key, value;

  if (id == NULL)
    return NULL;

  g_hash_table_iter_init (&iter, table);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      DebugPoint *point = value;

      if (g_strcmp0 (point->id, id) == 0)
	return point;
    }
  return NULL;
}

static void
enter_stopped (STCoreDebug * self, const gchar * reason, const gchar * id,
	       gdouble value, guint handle)
{
  STCoreDebugPrivate *priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  priv->paused = TRUE;
  g_free (priv->stop_reason);
  g_free (priv->stop_id);
  priv->stop_reason = g_strdup (reason);
  priv->stop_id = g_strdup (id ? id : "");
  priv->stop_value = value;
  priv->stop_handle = handle;

  LOGI ("debug-server Stopped reason=%s id=%s value=%g handle=%u", reason,
	priv->stop_id, value, handle);

  broadcast_signal (self, "Stopped",
		    g_variant_new ("(ssdu)", reason, priv->stop_id, value,
				   handle));
}

guint
st_core_debug_add_breakpoint (STCoreDebug * self, const gchar * id,
			      GError ** error)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), 0);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return add_point (priv->breakpoints, &priv->next_handle, id, error);
}

gboolean
st_core_debug_remove_breakpoint (STCoreDebug * self, guint handle,
				 GError ** error)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return remove_point (priv->breakpoints, handle, error);
}

guint
st_core_debug_add_watchpoint (STCoreDebug * self, const gchar * id,
			      GError ** error)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), 0);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return add_point (priv->watchpoints, &priv->next_handle, id, error);
}

gboolean
st_core_debug_remove_watchpoint (STCoreDebug * self, guint handle,
				 GError ** error)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return remove_point (priv->watchpoints, handle, error);
}

void
st_core_debug_continue (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  if (self == NULL || !ST_IS_CORE_DEBUG (self))
    return;

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  if (!priv->paused)
    return;

  LOGI ("debug-server Continue (was stopped: %s %s)",
	priv->stop_reason ? priv->stop_reason : "",
	priv->stop_id ? priv->stop_id : "");

  priv->paused = FALSE;
  g_clear_pointer (&priv->stop_reason, g_free);
  g_clear_pointer (&priv->stop_id, g_free);
  priv->stop_value = 0;
  priv->stop_handle = 0;
}

gboolean
st_core_debug_is_paused (STCoreDebug * self)
{
  STCoreDebugPrivate *priv;

  g_return_val_if_fail (ST_IS_CORE_DEBUG (self), FALSE);
  priv = ST_CORE_DEBUG_GET_PRIVATE (self);
  return priv->paused;
}

gboolean
st_core_debug_intercept_change (STCoreDebug * self, const gchar * id,
				gdouble value)
{
  STCoreDebugPrivate *priv;
  DebugPoint *watch;
  DebugPoint *brk;

  if (self == NULL || !ST_IS_CORE_DEBUG (self))
    return FALSE;

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  if (priv->paused)
    return TRUE;

  watch = find_point_by_id (priv->watchpoints, id);
  if (watch != NULL)
    {
      enter_stopped (self, "watchpoint", id, value, watch->handle);
      return TRUE;
    }

  brk = find_point_by_id (priv->breakpoints, id);
  if (brk != NULL)
    {
      enter_stopped (self, "breakpoint", id, value, brk->handle);
      return TRUE;
    }

  return FALSE;
}

gboolean
st_core_debug_intercept_apply (STCoreDebug * self, const gchar * id)
{
  STCoreDebugPrivate *priv;
  DebugPoint *brk;

  if (self == NULL || !ST_IS_CORE_DEBUG (self))
    return FALSE;

  priv = ST_CORE_DEBUG_GET_PRIVATE (self);

  if (priv->paused)
    return TRUE;

  /* id == NULL means full re-eval (peer ready); only honor pause. */
  if (id == NULL)
    return FALSE;

  brk = find_point_by_id (priv->breakpoints, id);
  if (brk != NULL)
    {
      enter_stopped (self, "breakpoint", id, 0, brk->handle);
      return TRUE;
    }

  return FALSE;
}
