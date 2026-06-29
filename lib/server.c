/*
 * libstarter-core - server.c
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <glib.h>
#include <gio/gio.h>

#include <starter/stats.h>
#include <starter/process-stats-collector.h>

#include <starter/mdns.h>

#include <starter/server.h>

#include <starter/input.h>
#include <starter/output.h>

#include "internals.h"

#define ST_SERVER_FIRST_ALLOC_PORT 8000
#define ST_SERVER_LAST_ALLOC_PORT 9000

typedef struct
{
  STStatsCollector *stats_coll;
  GDBusNodeInfo *introspection_data;
  GDBusServer *server;
  GPtrArray *connections;
  gchar *name;
  guint port;
  STVersion version;
  GPtrArray *inputs;
  GPtrArray *outputs;
  STStats *stats;
} STServerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (STServer, st_server, G_TYPE_OBJECT)
#define ST_SERVER_GET_PRIVATE(obj) \
    ((STServerPrivate *) st_server_get_instance_private (ST_SERVER (obj)))
     static const gchar introspection_xml[] =
       "<node name='/org/starter/service'>"
       "  <interface name='org.starter.Service'>"
       "    <method name='GetName'>"
       "      <arg direction='out' type='s' name='name'/>"
       "    </method>"
       "    <method name='GetVersion'>"
       "      <arg direction='out' type='(iii)' name='version'/>"
       "    </method>"
       "    <method name='GetInputs'>"
       "      <arg direction='out' type='a{s(siddddt)}' name='inputs'/>"
       "    </method>"
       "    <method name='GetOutputs'>"
       "      <arg direction='out' type='a{s(siddddt)}' name='ouputs'/>"
       "    </method>"
       "    <method name='SetOutput'>"
       "      <arg direction='in' type='s' name='id'/>"
       "      <arg direction='in' type='d' name='value'/>"
       "      <arg direction='in' type='t' name='flags'/>"
       "      <arg direction='out' type='i' name='error'/>"
       "    </method>"
       "    <method name='GetStats'>"
       "      <arg direction='out' type='(sddddddddddddddddddddddddddd)' name='stats'/>"
       "    </method>"
       "    <signal name='InputChanged'>"
       "      <arg name='object' type='{s(siddddt)}'/>"
       "    </signal>"
       "    <signal name='OutputChanged'>"
       "      <arg name='object' type='{s(siddddt)}'/>"
       "    </signal>"
       "    <signal name='StatsChanged'>"
       "      <arg name='object' type='(sddddddddddddddddddddddddddd)'/>"
       "    </signal>" "  </interface>" "</node>";

     static gpointer copy_st_input (gconstpointer src, gpointer user_data)
{
  return st_input_dup (src);
}

static gpointer
copy_st_output (gconstpointer src, gpointer user_data)
{
  return st_output_dup (src);
}

static void
emit_input_changed (GDBusConnection * connection, STInput * input)
{
  GVariant *item;
  GVariant *tuple;

  item = g_variant_new ("{s(siddddt)}",
			st_input_get_name (input),
			st_input_get_description (input),
			st_input_get_unit (input),
			st_input_get_min (input),
			st_input_get_max (input),
			st_input_get_step (input),
			st_input_get_val (input), st_input_get_flags (input));

  tuple = g_variant_new_tuple (&item, 1);

  g_dbus_connection_emit_signal (connection,
				 NULL,
				 "/org/starter/service",
				 "org.starter.Service",
				 "InputChanged", tuple, NULL);

}

static void
emit_output_changed (GDBusConnection * connection, STOutput * output)
{
  GVariant *item;
  GVariant *tuple;

  item = g_variant_new ("{s(siddddt)}",
			st_output_get_name (output),
			st_output_get_description (output),
			st_output_get_unit (output),
			st_output_get_min (output),
			st_output_get_max (output),
			st_output_get_step (output),
			st_output_get_val (output),
			st_output_get_flags (output));

  tuple = g_variant_new_tuple (&item, 1);

  g_dbus_connection_emit_signal (connection,
				 NULL,
				 "/org/starter/service",
				 "org.starter.Service",
				 "OutputChanged", tuple, NULL);
}

static void
emit_stats_changed (GDBusConnection * connection, STStats * stats)
{
  const gchar *name;
  STStatsEntry vmsize;
  STStatsEntry vmrss;
  STStatsEntry vmswap;
  STStatsEntry io_read;
  STStatsEntry io_write;
  STStatsEntry priority;
  STStatsEntry threads;
  STStatsEntry icswitch;
  STStatsEntry cpu;
  GVariant *tuple;

  st_stats_get_full (stats,
		     &name,
		     &vmsize,
		     &vmrss,
		     &vmswap,
		     &io_read,
		     &io_write, &priority, &threads, &icswitch, &cpu);

  tuple = g_variant_new ("(sddddddddddddddddddddddddddd)",
			 name,
			 vmsize.min, vmsize.max, vmsize.last,
			 vmrss.min, vmrss.max, vmrss.last,
			 vmswap.min, vmswap.max, vmswap.last,
			 io_read.min, io_read.max, io_read.last,
			 io_write.min, io_write.max, io_write.last,
			 priority.min, priority.max, priority.last,
			 threads.min, threads.max, threads.last,
			 icswitch.min, icswitch.max, icswitch.last,
			 cpu.min, cpu.max, cpu.last);

  g_dbus_connection_emit_signal (connection,
				 NULL,
				 "/org/starter/service",
				 "org.starter.Service",
				 "StatsChanged",
				 g_variant_new_tuple (&tuple, 1), NULL);
}

static void
input_changed_callback (STInput * self, gpointer user_data)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (user_data);
  guint i;

  for (i = 0; i < priv->connections->len; i++)
    emit_input_changed (priv->connections->pdata[i], self);
}

static void
output_changed_callback (STOutput * self, gpointer user_data)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (user_data);
  guint i;

  for (i = 0; i < priv->connections->len; i++)
    emit_output_changed (priv->connections->pdata[i], self);
}

static void
stats_changed_callback (STStats * self, gpointer user_data)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (user_data);
  guint i;

  LOGD ("emit stats changed signal");

  for (i = 0; i < priv->connections->len; i++)
    emit_stats_changed (priv->connections->pdata[i], self);
}

static void
setup_inputs_signals (STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);
  gint i;

  for (i = 0; i < priv->inputs->len; i++)
    g_signal_connect (priv->inputs->pdata[i], "changed",
		      G_CALLBACK (input_changed_callback), self);
}

static void
setup_outputs_signals (STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);
  gint i;

  for (i = 0; i < priv->outputs->len; i++)
    g_signal_connect (priv->outputs->pdata[i], "changed",
		      G_CALLBACK (output_changed_callback), self);
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
  GError *error = NULL;
  STServer *self = user_data;
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  LOGD ("handle method call %s", method_name);

  if (g_strcmp0 (method_name, "GetName") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(s)",
							    g_strdup (priv->
								      name)));
    }
  else if (g_strcmp0 (method_name, "GetVersion") == 0)
    {
      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(iii)",
							    priv->version.
							    major,
							    priv->version.
							    minor,
							    priv->version.
							    micro));
    }
  else if (g_strcmp0 (method_name, "GetInputs") == 0)
    {
      const GPtrArray *inputs = priv->inputs;
      guint i;

      GVariantBuilder builder;
      GVariant *map_variant;
      GVariant *tuple;

      g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{s(siddddt)}"));

      for (i = 0; i < inputs->len; i++)
	{
	  const STInput *in = inputs->pdata[i];

	  g_variant_builder_add (&builder,
				 "{s(siddddt)}",
				 st_input_get_name (in),
				 st_input_get_description (in),
				 st_input_get_unit (in),
				 st_input_get_min (in),
				 st_input_get_max (in),
				 st_input_get_step (in),
				 st_input_get_val (in),
				 st_input_get_flags (in));
	}

      map_variant = g_variant_builder_end (&builder);
      tuple = g_variant_new_tuple (&map_variant, 1);

      g_dbus_method_invocation_return_value (invocation, tuple);
    }
  else if (g_strcmp0 (method_name, "GetOutputs") == 0)
    {
      const GPtrArray *outputs = priv->outputs;
      guint i;

      GVariantBuilder builder;
      GList *outputs_iter;
      GVariant *map_variant;
      GVariant *tuple;

      g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{s(siddddt)}"));

      for (i = 0; i < outputs->len; i++)
	{
	  const STOutput *out = outputs->pdata[i];

	  g_variant_builder_add (&builder,
				 "{s(siddddt)}",
				 st_output_get_name (out),
				 st_output_get_description (out),
				 st_output_get_unit (out),
				 st_output_get_min (out),
				 st_output_get_max (out),
				 st_output_get_step (out),
				 st_output_get_val (out),
				 st_output_get_flags (out));
	}

      map_variant = g_variant_builder_end (&builder);
      tuple = g_variant_new_tuple (&map_variant, 1);

      g_dbus_method_invocation_return_value (invocation, tuple);
    }
  else if (g_strcmp0 (method_name, "SetOutput") == 0)
    {
      const gchar *name;
      double value;
      guint64 flags;
      guint i;
      gboolean res = FALSE;

      g_variant_get (parameters, "(&sdt)", &name, &value, &flags);

      for (i = 0; i < priv->outputs->len && res == FALSE; i++)
	{
	  STOutput *out = priv->outputs->pdata[i];

	  if (strcmp (name, st_output_get_name (out)) == 0)
	    {
	      LOGD ("found output at index %u", i);

	      res = st_output_set_val (out, value, &error)
		&& st_output_set_flags (out, flags, &error);

	      LOGD ("result: %d", res);

	      break;
	    }
	}

      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new ("(i)",
							    (int) res));
    }
  else if (g_strcmp0 (method_name, "GetStats") == 0)
    {
      const gchar *name;
      STStatsEntry vmsize;
      STStatsEntry vmrss;
      STStatsEntry vmswap;
      STStatsEntry io_read;
      STStatsEntry io_write;
      STStatsEntry priority;
      STStatsEntry threads;
      STStatsEntry icswitch;
      STStatsEntry cpu;

      GVariant *tuple;

      st_stats_get_full (priv->stats,
			 &name,
			 &vmsize,
			 &vmrss,
			 &vmswap,
			 &io_read,
			 &io_write, &priority, &threads, &icswitch, &cpu);

      tuple = g_variant_new ("(sddddddddddddddddddddddddddd)",
			     name,
			     vmsize.min, vmsize.max, vmsize.last,
			     vmrss.min, vmrss.max, vmrss.last,
			     vmswap.min, vmswap.max, vmswap.last,
			     io_read.min, io_read.max, io_read.last,
			     io_write.min, io_write.max, io_write.last,
			     priority.min, priority.max, priority.last,
			     threads.min, threads.max, threads.last,
			     icswitch.min, icswitch.max, icswitch.last,
			     cpu.min, cpu.max, cpu.last);

      g_dbus_method_invocation_return_value (invocation,
					     g_variant_new_tuple (&tuple, 1));
    }
  else
    {
      LOGE ("unmanaged method");

      g_set_error (&error, G_DBUS_ERROR, G_IO_ERROR_NOT_FOUND,
		   "unknown method: %s", method_name);
    }

  if (error)
    {
      LOGD ("error in %s: %s", method_name, error->message);

      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);
    }
}

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  NULL,
  NULL,
  {0}
};

static void
connection_closed (GDBusConnection * connection,
		   gboolean remote_peer_vanished,
		   GError * Error, gpointer user_data)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (user_data);

  LOGD ("Client disconnected.");

  g_ptr_array_remove (priv->connections, connection);
}

static gboolean
on_new_connection (GDBusServer * server,
		   GDBusConnection * connection, gpointer user_data)
{
  STServer *self = user_data;
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);
  guint registration_id;

  GCredentials *credentials;
  gchar *s;

  LOGD ("on new connection");

  credentials = g_dbus_connection_get_peer_credentials (connection);
  if (credentials == NULL)
    s = g_strdup ("(no credentials received)");
  else
    s = g_credentials_to_string (credentials);

  LOGD
    ("Client connected. Credentials: %s, Negotiated capabilities: unix-fd-passing=%d",
     s,
     g_dbus_connection_get_capabilities (connection) &
     G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);

  g_ptr_array_add (priv->connections, connection);

  g_object_ref (connection);

  g_signal_connect (connection, "closed", G_CALLBACK (connection_closed),
		    self);

  registration_id = g_dbus_connection_register_object (connection, "/org/starter/service", priv->introspection_data->interfaces[0], &interface_vtable, self,	/* user_data */
						       NULL,	/* user_data_free_func */
						       NULL);	/* GError** */
  g_assert (registration_id > 0);
  g_free (s);
  return TRUE;
}

static void
st_server_finalize (GObject * object)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (object);

  LOGD ("finalize STServer %p", object);

  g_clear_object(&priv->stats);
  g_clear_object(&priv->stats_coll);
  g_dbus_node_info_unref (priv->introspection_data);
  g_ptr_array_unref (priv->connections);

  if(priv->server)
    {
      g_dbus_server_stop (priv->server);
      g_clear_object(&priv->server);
    }

  g_free (priv->name);

  if(priv->inputs)
    g_ptr_array_unref (priv->inputs);

  if(priv->outputs)
    g_ptr_array_unref (priv->outputs);

  G_OBJECT_CLASS (st_server_parent_class)->finalize (object);
}

static void
st_server_class_init (STServerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = st_server_finalize;
}

static void
st_server_init (STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  priv->stats = NULL;
  priv->stats_coll = NULL;

  priv->introspection_data =
    g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  priv->server = NULL;
  priv->connections = g_ptr_array_new_full (0, g_object_unref);
  priv->name = NULL;
  priv->port = 0;
  memset (&priv->version, 0x00, sizeof priv->version);
  priv->inputs = NULL;
  priv->outputs = NULL;
}

STServer *
st_server_new (const gchar * name, const STVersion * version,
	       GPtrArray * inputs, GPtrArray * outputs)
{
  STServer *r;
  STServerPrivate *priv;

  r = g_object_new (ST_TYPE_SERVER, NULL);
  priv = ST_SERVER_GET_PRIVATE (r);

  priv->name = g_strdup (name);
  memcpy (&priv->version, version, sizeof (STVersion));
  priv->inputs = inputs;
  priv->outputs = outputs;

  g_ptr_array_ref (inputs);
  g_ptr_array_ref (outputs);

  setup_inputs_signals (r);
  setup_outputs_signals (r);

  priv->stats = st_stats_new (name);
  g_signal_connect (priv->stats, "changed",
		    G_CALLBACK (stats_changed_callback), r);

  priv->stats_coll =
    ST_STATS_COLLECTOR (st_process_stats_collector_new (priv->stats, 1000));

  LOGD ("Created server with name: %s, inputs: %p, outputs: %p", priv->name,
	priv->inputs, priv->outputs);

  return r;
}

const gchar *
st_server_get_name (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return priv->name;
}

gchar *
st_server_dup_name (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return g_strdup (priv->name);
}

gboolean
st_server_set_name (STServer * self, const gchar * name, GError ** error)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  g_free (priv->name);
  priv->name = g_strdup (name);

  return TRUE;
}

const STVersion *
st_server_get_version (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return &priv->version;
}

STVersion *
st_server_dup_version (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return g_memdup2 (&priv->version, sizeof (STVersion));
}

gboolean
st_server_set_version (STServer * self, const STVersion * version,
		       GError ** error)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  memcpy (&priv->version, version, sizeof (STVersion));

  return TRUE;
}

const GPtrArray *
st_server_get_inputs (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return priv->inputs;
}

GPtrArray *
st_server_dup_inputs (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return g_ptr_array_copy (priv->inputs, copy_st_input, NULL);
}

gboolean
st_server_set_inputs (STServer * self, GPtrArray * inputs)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  g_object_unref (priv->inputs);

  priv->inputs = inputs;

  g_object_ref (inputs);

  setup_inputs_signals (self);

  return TRUE;
}

const GPtrArray *
st_server_get_outputs (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return priv->outputs;
}

GPtrArray *
st_server_dup_outputs (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return g_ptr_array_copy (priv->inputs, copy_st_input, NULL);
}

gboolean
st_server_set_outputs (STServer * self, GPtrArray * outputs, GError * error)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);

  g_object_unref (priv->outputs);
  priv->outputs = outputs;

  g_object_ref (outputs);

  setup_outputs_signals (self);

  return TRUE;
}

gboolean
st_server_start (STServer * self, guint16 port, GError ** error)
{
  gboolean r = FALSE;
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE (self);
  gchar *guid;
  char b[0x20];
  const gchar *addr = NULL;

  guid = g_dbus_generate_guid ();

  snprintf (b, sizeof b, "tcp:host=0.0.0.0,port=%hu", port);

  LOGD ("try create new dbus server with address '%s'", b);

  priv->port = port;

  priv->server = g_dbus_server_new_sync (b,
					 G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS,
					 guid, NULL, NULL, error);

  g_free(guid);

  if (priv->server)
    {
      const gchar *addr;

      addr = g_dbus_server_get_client_address(priv->server);

      if(addr)
	{
	  const gchar *port;

	  port = strstr(addr, "port=");

	  if(port == NULL || sscanf(port + 0x05, "%u", &priv->port) != 1)
	    LOGW("error parsing local bind address!");
	}
      else
	LOGW("error retrieve local bind address!");

      g_signal_connect (priv->server, "new-connection",
			G_CALLBACK (on_new_connection), self);

      g_dbus_server_start (priv->server);

      r = TRUE;
    }

  return r;
}

guint16
st_server_get_port (const STServer * self)
{
  STServerPrivate *priv = ST_SERVER_GET_PRIVATE ((STServer *) self);

  return priv->port;
}
