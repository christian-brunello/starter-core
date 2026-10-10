/*
 * starter-core - test-debug-server.c
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

#include <glib.h>
#include <gio/gio.h>

#include "internals.h"
#include "debug-server.h"

#include <starter/defs.h>
#include <starter/error.h>
#include <starter/server.h>
#include <starter/client.h>
#include <starter/version.h>
#include <starter/input.h>
#include <starter/output.h>

#define DEBUG_OBJECT_PATH "/org/starter/core/debug"
#define DEBUG_INTERFACE "org.starter.Core.Debug"

/* Provided by main.c in the daemon; required by linked src objects. */
VerboseLevel verbose_level = VERBOSE_LEVEL_NULL;

typedef struct
{
  STCore core;
  STCoreDebug *debug;
  GMainContext *context;
  GMainLoop *loop;
  GThread *thread;
  GMutex mutex;
  GCond cond;
  gboolean ready;
  gboolean start_ok;
  GError *start_error;

  /* signal capture (client-side) */
  guint peer_signals;
  gchar *last_peer_name;
  gchar *last_peer_addr;
  guint last_peer_port;
  guint last_peer_state;
  gchar *last_peer_event;
  guint input_signals;
  gchar *last_input_name;
  gdouble last_input_val;
  guint rule_signals;
  gchar *last_rule_dest;
  gchar *last_rule_describe;
  guint trigger_signals;
  gchar *last_trigger_dest;
  gchar *last_trigger_describe;

  /* optional I/O peer for SetInput/SetOutput success tests */
  STServer *io_server;
  GPtrArray *io_inputs;
  GPtrArray *io_outputs;
} Fixture;

static void
core_init_empty (STCore * core)
{
  memset (core, 0, sizeof (*core));
  core->srvmatch = g_strdup ("^(STARTER:|ST:).*");
  core->rules_file = g_strdup ("/tmp/starter-debug-server-test.rules");
  core->clients =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  core->all_inputs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  core->all_outputs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

static void
core_clear (STCore * core)
{
  g_clear_pointer (&core->clients, g_hash_table_unref);
  g_clear_pointer (&core->pending_clients, g_hash_table_unref);
  g_clear_pointer (&core->all_inputs, g_hash_table_unref);
  g_clear_pointer (&core->all_outputs, g_hash_table_unref);
  g_clear_pointer (&core->srvmatch, g_free);
  g_clear_pointer (&core->rules_file, g_free);
  g_clear_pointer (&core->debug_server_bind, g_free);
}

/*
 * GDBusServer attaches sources to the thread-default main context.
 * Own a private context+loop on a worker thread so sync client calls
 * from the test thread do not deadlock.
 */
static gpointer
server_thread (gpointer data)
{
  Fixture *f = data;

  g_main_context_push_thread_default (f->context);

  f->start_ok =
    st_core_debug_start (f->debug, "127.0.0.1:0", &f->start_error);

  g_mutex_lock (&f->mutex);
  f->ready = TRUE;
  g_cond_signal (&f->cond);
  g_mutex_unlock (&f->mutex);

  if (f->start_ok)
    g_main_loop_run (f->loop);

  if (f->debug)
    st_core_debug_stop (f->debug);

  g_main_context_pop_thread_default (f->context);
  return NULL;
}

static void
fixture_setup (Fixture * f, gconstpointer user_data)
{
  (void) user_data;

  memset (f, 0, sizeof (*f));
  g_mutex_init (&f->mutex);
  g_cond_init (&f->cond);

  core_init_empty (&f->core);
  f->debug = st_core_debug_new (&f->core);
  g_assert_nonnull (f->debug);

  f->context = g_main_context_new ();
  f->loop = g_main_loop_new (f->context, FALSE);
  f->thread = g_thread_new ("debug-server-test", server_thread, f);

  g_mutex_lock (&f->mutex);
  while (!f->ready)
    g_cond_wait (&f->cond, &f->mutex);
  g_mutex_unlock (&f->mutex);

  g_assert_true (f->start_ok);
  g_assert_no_error (f->start_error);
  g_assert_true (st_core_debug_is_active (f->debug));
  g_assert_cmpuint (st_core_debug_get_bind_port (f->debug), >, 0);
}

static void
fixture_teardown (Fixture * f, gconstpointer user_data)
{
  (void) user_data;

  if (f->loop)
    g_main_loop_quit (f->loop);
  if (f->thread)
    g_thread_join (f->thread);

  g_clear_pointer (&f->loop, g_main_loop_unref);
  g_clear_pointer (&f->context, g_main_context_unref);
  g_clear_object (&f->debug);
  g_clear_error (&f->start_error);
  g_clear_pointer (&f->last_peer_name, g_free);
  g_clear_pointer (&f->last_peer_addr, g_free);
  g_clear_pointer (&f->last_peer_event, g_free);
  g_clear_pointer (&f->last_input_name, g_free);
  g_clear_pointer (&f->last_rule_dest, g_free);
  g_clear_pointer (&f->last_rule_describe, g_free);
  g_clear_pointer (&f->last_trigger_dest, g_free);
  g_clear_pointer (&f->last_trigger_describe, g_free);
  g_clear_object (&f->io_server);
  g_clear_pointer (&f->io_inputs, g_ptr_array_unref);
  g_clear_pointer (&f->io_outputs, g_ptr_array_unref);
  core_clear (&f->core);
  g_mutex_clear (&f->mutex);
  g_cond_clear (&f->cond);
}

static gboolean
iterate_until (gboolean (*pred) (gpointer), gpointer data, guint timeout_ms)
{
  gint64 end = g_get_monotonic_time () + (gint64) timeout_ms * 1000;

  while (!pred (data) && g_get_monotonic_time () < end)
    g_main_context_iteration (NULL, TRUE);

  return pred (data);
}

static gboolean
peer_signal_seen (gpointer data)
{
  return ((Fixture *) data)->peer_signals > 0;
}

static gboolean
input_signal_seen (gpointer data)
{
  return ((Fixture *) data)->input_signals > 0;
}

static gboolean
rule_signal_seen (gpointer data)
{
  return ((Fixture *) data)->rule_signals > 0;
}

static gboolean
trigger_signal_seen (gpointer data)
{
  return ((Fixture *) data)->trigger_signals > 0;
}

static void
on_peer_changed_signal (GDBusConnection * connection,
			const gchar * sender_name,
			const gchar * object_path,
			const gchar * interface_name,
			const gchar * signal_name, GVariant * parameters,
			gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *name = NULL;
  const gchar *addr = NULL;
  const gchar *event = NULL;
  guint port = 0;
  guint state = 0;

  (void) connection;
  (void) sender_name;
  (void) object_path;
  (void) interface_name;
  (void) signal_name;

  g_variant_get (parameters, "(&s&suu&s)", &name, &addr, &port, &state,
		 &event);
  f->peer_signals++;
  g_free (f->last_peer_name);
  g_free (f->last_peer_addr);
  g_free (f->last_peer_event);
  f->last_peer_name = g_strdup (name);
  f->last_peer_addr = g_strdup (addr);
  f->last_peer_event = g_strdup (event);
  f->last_peer_port = port;
  f->last_peer_state = state;
}

static void
on_input_changed_signal (GDBusConnection * connection,
			 const gchar * sender_name,
			 const gchar * object_path,
			 const gchar * interface_name,
			 const gchar * signal_name, GVariant * parameters,
			 gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *name = NULL;
  const gchar *desc = NULL;
  gint unit = 0;
  gdouble min = 0, max = 0, step = 0, val = 0;
  guint64 flags = 0;

  (void) connection;
  (void) sender_name;
  (void) object_path;
  (void) interface_name;
  (void) signal_name;

  g_variant_get (parameters, "({&s(&siddddt)})", &name, &desc, &unit, &min,
		 &max, &step, &val, &flags);
  f->input_signals++;
  g_free (f->last_input_name);
  f->last_input_name = g_strdup (name);
  f->last_input_val = val;
}

static void
on_rule_triggered_signal (GDBusConnection * connection,
			  const gchar * sender_name,
			  const gchar * object_path,
			  const gchar * interface_name,
			  const gchar * signal_name, GVariant * parameters,
			  gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *dest = NULL;
  const gchar *describe = NULL;

  (void) connection;
  (void) sender_name;
  (void) object_path;
  (void) interface_name;
  (void) signal_name;

  g_variant_get (parameters, "(&s&s)", &dest, &describe);
  f->rule_signals++;
  g_free (f->last_rule_dest);
  g_free (f->last_rule_describe);
  f->last_rule_dest = g_strdup (dest);
  f->last_rule_describe = g_strdup (describe);
}

static void
on_trigger_triggered_signal (GDBusConnection * connection,
			     const gchar * sender_name,
			     const gchar * object_path,
			     const gchar * interface_name,
			     const gchar * signal_name, GVariant * parameters,
			     gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *dest = NULL;
  const gchar *describe = NULL;

  (void) connection;
  (void) sender_name;
  (void) object_path;
  (void) interface_name;
  (void) signal_name;

  g_variant_get (parameters, "(&s&s)", &dest, &describe);
  f->trigger_signals++;
  g_free (f->last_trigger_dest);
  g_free (f->last_trigger_describe);
  f->last_trigger_dest = g_strdup (dest);
  f->last_trigger_describe = g_strdup (describe);
}

static GDBusConnection *
connect_debug (Fixture * f, GError ** error)
{
  g_autofree gchar *addr = NULL;
  GDBusConnection *conn;
  GVariant *reply;

  addr =
    g_strdup_printf ("tcp:host=127.0.0.1,port=%hu",
		     st_core_debug_get_bind_port (f->debug));

  conn =
    g_dbus_connection_new_for_address_sync (addr,
					    G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
					    NULL, NULL, error);
  if (conn == NULL)
    return NULL;

  /*
   * Barrier: GetStatus only succeeds after on_new_connection has
   * registered the object on the server thread.
   */
  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "GetStatus", NULL,
				 G_VARIANT_TYPE ("(a{sv})"),
				 G_DBUS_CALL_FLAGS_NONE, 5000, NULL, error);
  if (reply == NULL)
    {
      g_object_unref (conn);
      return NULL;
    }
  g_variant_unref (reply);
  return conn;
}

static void
test_debug_server_bind_parse_errors (void)
{
  STCore core;
  STCoreDebug *debug;
  GError *error = NULL;

  core_init_empty (&core);
  debug = st_core_debug_new (&core);

  g_assert_false (st_core_debug_start (debug, "no-port", &error));
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_clear_error (&error);

  g_assert_false (st_core_debug_start (debug, "127.0.0.1:99999", &error));
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_clear_error (&error);

  g_assert_false (st_core_debug_start (debug, ":17373", &error));
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_clear_error (&error);

  g_object_unref (debug);
  core_clear (&core);
}

static void
test_debug_server_start_stop (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;

  (void) user_data;

  g_assert_cmpstr (st_core_debug_get_bind_host (f->debug), ==, "127.0.0.1");
  g_assert_nonnull (st_core_debug_get_client_address (f->debug));

  /* Second start must fail while active (safe from any thread). */
  g_assert_false (st_core_debug_start
		  (f->debug, "127.0.0.1:0", &error));
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_EXISTS);
  g_clear_error (&error);
}

static void
test_debug_server_get_status (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;
  GVariant *status;
  GVariant *v;
  const gchar *s;
  guint32 u32;
  guint16 u16;
  gboolean b;

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "GetStatus", NULL,
				 G_VARIANT_TYPE ("(a{sv})"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_assert_nonnull (reply);

  status = g_variant_get_child_value (reply, 0);

  v = g_variant_lookup_value (status, "package", G_VARIANT_TYPE_STRING);
  g_assert_nonnull (v);
  s = g_variant_get_string (v, NULL);
  g_assert_cmpstr (s, ==, PACKAGE);
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "version", G_VARIANT_TYPE_STRING);
  g_assert_nonnull (v);
  s = g_variant_get_string (v, NULL);
  g_assert_cmpstr (s, ==, VERSION);
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "rules-file", G_VARIANT_TYPE_STRING);
  g_assert_nonnull (v);
  s = g_variant_get_string (v, NULL);
  g_assert_cmpstr (s, ==, f->core.rules_file);
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "bind-host", G_VARIANT_TYPE_STRING);
  g_assert_nonnull (v);
  s = g_variant_get_string (v, NULL);
  g_assert_cmpstr (s, ==, "127.0.0.1");
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "bind-port", G_VARIANT_TYPE_UINT16);
  g_assert_nonnull (v);
  u16 = g_variant_get_uint16 (v);
  g_assert_cmpuint (u16, ==, st_core_debug_get_bind_port (f->debug));
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "peers", G_VARIANT_TYPE_UINT32);
  g_assert_nonnull (v);
  u32 = g_variant_get_uint32 (v);
  g_assert_cmpuint (u32, ==, 0);
  g_variant_unref (v);

  v = g_variant_lookup_value (status, "no-discovery", G_VARIANT_TYPE_BOOLEAN);
  g_assert_nonnull (v);
  b = g_variant_get_boolean (v);
  g_assert_false (b);
  g_variant_unref (v);

  g_variant_unref (status);
  g_variant_unref (reply);
  g_object_unref (conn);
}

static void
test_debug_server_list_peers (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;
  GVariant *peers;
  GVariantIter iter;
  const gchar *name;
  const gchar *host;
  guint port;
  guint state;
  guint count = 0;

  (void) user_data;

  st_core_debug_emit_peer_changed (f->debug, "ST:TestPeer", "10.0.0.5",
				   4242, (guint) ST_CLIENT_STATE_READY,
				   "ready");

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListPeers", NULL,
				 G_VARIANT_TYPE ("(a(ssuu))"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_assert_nonnull (reply);

  peers = g_variant_get_child_value (reply, 0);
  g_variant_iter_init (&iter, peers);
  while (g_variant_iter_next (&iter, "(&s&suu)", &name, &host, &port, &state))
    {
      count++;
      g_assert_cmpstr (name, ==, "ST:TestPeer");
      g_assert_cmpstr (host, ==, "10.0.0.5");
      g_assert_cmpuint (port, ==, 4242);
      g_assert_cmpuint (state, ==, (guint) ST_CLIENT_STATE_READY);
    }
  g_assert_cmpuint (count, ==, 1);

  g_variant_unref (peers);
  g_variant_unref (reply);
  g_object_unref (conn);
}

typedef struct
{
  Fixture *f;
  gboolean done;
  /* PeerChanged */
  const gchar *peer_name;
  const gchar *peer_addr;
  guint16 peer_port;
  guint peer_state;
  const gchar *peer_event;
  /* InputChanged */
  STInput *input;
  /* RuleTriggered / TriggerTriggered */
  const gchar *rule_dest;
  const gchar *rule_describe;
  const gchar *trigger_dest;
  const gchar *trigger_describe;
} ServerEmitJob;

static gboolean
emit_peer_on_server (gpointer data)
{
  ServerEmitJob *job = data;

  st_core_debug_emit_peer_changed (job->f->debug, job->peer_name,
				   job->peer_addr, job->peer_port,
				   job->peer_state, job->peer_event);
  job->done = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
emit_input_on_server (gpointer data)
{
  ServerEmitJob *job = data;

  st_core_debug_emit_input_changed (job->f->debug, job->input);
  job->done = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
emit_rule_on_server (gpointer data)
{
  ServerEmitJob *job = data;

  st_core_debug_emit_rule_triggered (job->f->debug, job->rule_dest,
				     job->rule_describe);
  job->done = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
emit_trigger_on_server (gpointer data)
{
  ServerEmitJob *job = data;

  st_core_debug_emit_trigger_triggered (job->f->debug, job->trigger_dest,
					job->trigger_describe);
  job->done = TRUE;
  return G_SOURCE_REMOVE;
}

static void
wait_server_job (Fixture * f, ServerEmitJob * job)
{
  gint64 end = g_get_monotonic_time () + 2 * G_TIME_SPAN_SECOND;

  while (!job->done && g_get_monotonic_time () < end)
    {
      g_main_context_iteration (NULL, FALSE);
      g_thread_yield ();
    }
  g_assert_true (job->done);
  (void) f;
}

static void
test_debug_server_peer_signal (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  guint sid;
  ServerEmitJob job = { 0 };

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  sid =
    g_dbus_connection_signal_subscribe (conn, NULL, DEBUG_INTERFACE,
					"PeerChanged", DEBUG_OBJECT_PATH,
					NULL, G_DBUS_SIGNAL_FLAGS_NONE,
					on_peer_changed_signal, f, NULL);
  g_assert_cmpuint (sid, !=, 0);

  f->peer_signals = 0;
  job.f = f;
  job.peer_name = "ST:Beep";
  job.peer_addr = "192.168.1.9";
  job.peer_port = 5555;
  job.peer_state = (guint) ST_CLIENT_STATE_PENDING;
  job.peer_event = "connecting";
  g_main_context_invoke (f->context, emit_peer_on_server, &job);
  wait_server_job (f, &job);

  g_assert_true (iterate_until (peer_signal_seen, f, 2000));
  g_assert_cmpstr (f->last_peer_name, ==, "ST:Beep");
  g_assert_cmpstr (f->last_peer_addr, ==, "192.168.1.9");
  g_assert_cmpuint (f->last_peer_port, ==, 5555);
  g_assert_cmpuint (f->last_peer_state, ==,
		    (guint) ST_CLIENT_STATE_PENDING);
  g_assert_cmpstr (f->last_peer_event, ==, "connecting");

  g_dbus_connection_signal_unsubscribe (conn, sid);
  g_object_unref (conn);
}

static void
test_debug_server_input_signal (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  guint sid;
  STInput *in;
  ServerEmitJob job = { 0 };

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  sid =
    g_dbus_connection_signal_subscribe (conn, NULL, DEBUG_INTERFACE,
					"InputChanged", DEBUG_OBJECT_PATH,
					NULL, G_DBUS_SIGNAL_FLAGS_NONE,
					on_input_changed_signal, f, NULL);
  g_assert_cmpuint (sid, !=, 0);

  in =
    st_input_new ("time:second", "seconds", ST_UNIT_NULL, 0, 59, 1, 42, 0);
  f->input_signals = 0;
  job.f = f;
  job.input = in;
  g_main_context_invoke (f->context, emit_input_on_server, &job);
  wait_server_job (f, &job);

  g_assert_true (iterate_until (input_signal_seen, f, 2000));
  g_assert_cmpstr (f->last_input_name, ==, "time:second");
  g_assert_cmpfloat (f->last_input_val, ==, 42.0);

  g_object_unref (in);
  g_dbus_connection_signal_unsubscribe (conn, sid);
  g_object_unref (conn);
}

static void
test_debug_server_rule_trigger_signals (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  guint sid_rule;
  guint sid_trigger;
  ServerEmitJob job = { 0 };

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  sid_rule =
    g_dbus_connection_signal_subscribe (conn, NULL, DEBUG_INTERFACE,
					"RuleTriggered", DEBUG_OBJECT_PATH,
					NULL, G_DBUS_SIGNAL_FLAGS_NONE,
					on_rule_triggered_signal, f, NULL);
  sid_trigger =
    g_dbus_connection_signal_subscribe (conn, NULL, DEBUG_INTERFACE,
					"TriggerTriggered", DEBUG_OBJECT_PATH,
					NULL, G_DBUS_SIGNAL_FLAGS_NONE,
					on_trigger_triggered_signal, f, NULL);
  g_assert_cmpuint (sid_rule, !=, 0);
  g_assert_cmpuint (sid_trigger, !=, 0);

  f->rule_signals = 0;
  job.f = f;
  job.rule_dest = "beep:status";
  job.rule_describe =
    "SET 1.000000 WHEN (time:hour) == (7.000000)";
  g_main_context_invoke (f->context, emit_rule_on_server, &job);
  wait_server_job (f, &job);

  g_assert_true (iterate_until (rule_signal_seen, f, 2000));
  g_assert_cmpstr (f->last_rule_dest, ==, "beep:status");
  g_assert_true (g_str_has_prefix (f->last_rule_describe, "SET "));

  f->trigger_signals = 0;
  job.done = FALSE;
  job.trigger_dest = "alarm_active";
  job.trigger_describe =
    "TRIGGER WHEN (oil_is_cheap) == (1.000000) SET alarm_active = 1.000000";
  g_main_context_invoke (f->context, emit_trigger_on_server, &job);
  wait_server_job (f, &job);

  g_assert_true (iterate_until (trigger_signal_seen, f, 2000));
  g_assert_cmpstr (f->last_trigger_dest, ==, "alarm_active");
  g_assert_true (g_str_has_prefix
		 (f->last_trigger_describe, "TRIGGER WHEN"));

  g_dbus_connection_signal_unsubscribe (conn, sid_rule);
  g_dbus_connection_signal_unsubscribe (conn, sid_trigger);
  g_object_unref (conn);
}

static void
test_debug_server_set_unknown (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;
  gint ok = -1;

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "SetInput",
				 g_variant_new ("(sdt)", "missing:in", 1.0,
						(guint64) 0),
				 G_VARIANT_TYPE ("(i)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(i)", &ok);
  g_assert_cmpint (ok, ==, 0);
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "SetOutput",
				 g_variant_new ("(sdt)", "missing:out", 1.0,
						(guint64) 0),
				 G_VARIANT_TYPE ("(i)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(i)", &ok);
  g_assert_cmpint (ok, ==, 0);
  g_variant_unref (reply);

  g_assert_false (st_core_debug_set_input
		  (f->debug, "missing:in", 1.0, 0, &error));
  g_assert_error (error, ST_ERROR, ST_ERROR_UNDEFINED_IDENTIFIER);
  g_clear_error (&error);

  g_object_unref (conn);
}

typedef struct
{
  Fixture *f;
  GMainContext *context;
  GMainLoop *loop;
  GThread *thread;
  GMutex mutex;
  GCond cond;
  gboolean ready;
  gboolean ok;
  GError *error;
  guint16 port;
} IoPeer;

static gpointer
io_peer_thread (gpointer data)
{
  IoPeer *peer = data;
  STVersion version = { 1, 0, 0 };
  Fixture *f = peer->f;

  g_main_context_push_thread_default (peer->context);

  f->io_inputs = g_ptr_array_new_with_free_func (g_object_unref);
  f->io_outputs = g_ptr_array_new_with_free_func (g_object_unref);
  g_ptr_array_add (f->io_inputs,
		   st_input_new ("test:in", "in", ST_UNIT_NULL, 0, 100, 1,
				 0, 0));
  g_ptr_array_add (f->io_outputs,
		   st_output_new ("test:out", "out", ST_UNIT_NULL, 0, 100, 1,
				  0, 0));

  f->io_server =
    st_server_new ("ST:DebugIo", &version, f->io_inputs, f->io_outputs);
  peer->ok = st_server_start (f->io_server, 0, &peer->error);
  if (peer->ok)
    peer->port = st_server_get_port (f->io_server);

  g_mutex_lock (&peer->mutex);
  peer->ready = TRUE;
  g_cond_signal (&peer->cond);
  g_mutex_unlock (&peer->mutex);

  if (peer->ok)
    g_main_loop_run (peer->loop);

  g_main_context_pop_thread_default (peer->context);
  return NULL;
}

static void
test_debug_server_set_input_output (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;
  gint ok = -1;
  STClient *client;
  IoPeer peer = { 0 };
  const GPtrArray *inputs;
  const GPtrArray *outputs;

  (void) user_data;

  /*
   * I/O STServer on its own thread/context so SetOutput (handled on the
   * debug listen thread) can sync-call without deadlocking.
   */
  peer.f = f;
  g_mutex_init (&peer.mutex);
  g_cond_init (&peer.cond);
  peer.context = g_main_context_new ();
  peer.loop = g_main_loop_new (peer.context, FALSE);
  peer.thread = g_thread_new ("debug-io-peer", io_peer_thread, &peer);

  g_mutex_lock (&peer.mutex);
  while (!peer.ready)
    g_cond_wait (&peer.cond, &peer.mutex);
  g_mutex_unlock (&peer.mutex);

  g_assert_true (peer.ok);
  g_assert_no_error (peer.error);
  g_assert_cmpuint (peer.port, >, 0);

  client = st_client_new ();
  g_assert_true (st_client_start (client, "127.0.0.1", peer.port, &error));
  g_assert_no_error (error);

  g_hash_table_insert (f->core.clients, g_strdup ("ST:DebugIo"), client);
  g_hash_table_unref (f->core.all_inputs);
  g_hash_table_unref (f->core.all_outputs);
  f->core.all_inputs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  f->core.all_outputs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  {
    const GPtrArray *c_in = st_client_get_inputs (client);
    const GPtrArray *c_out = st_client_get_outputs (client);
    ClientInputMap *im = g_malloc0 (sizeof (*im));
    ClientOutputMap *om = g_malloc0 (sizeof (*om));

    im->client = client;
    im->index = 0;
    om->client = client;
    om->index = 0;
    g_hash_table_insert (f->core.all_inputs,
			 g_strdup (st_input_get_name (c_in->pdata[0])), im);
    g_hash_table_insert (f->core.all_outputs,
			 g_strdup (st_output_get_name (c_out->pdata[0])), om);
  }

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "SetInput",
				 g_variant_new ("(sdt)", "test:in", 7.0,
						(guint64) 0),
				 G_VARIANT_TYPE ("(i)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(i)", &ok);
  g_assert_cmpint (ok, ==, 1);
  g_variant_unref (reply);

  inputs = st_client_get_inputs (client);
  g_assert_cmpfloat (st_input_get_val (inputs->pdata[0]), ==, 7.0);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "SetOutput",
				 g_variant_new ("(sdt)", "test:out", 9.0,
						(guint64) 0),
				 G_VARIANT_TYPE ("(i)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(i)", &ok);
  g_assert_cmpint (ok, ==, 1);
  g_variant_unref (reply);

  {
    gint64 end = g_get_monotonic_time () + 2 * G_TIME_SPAN_SECOND;

    outputs = st_client_get_outputs (client);
    while (st_output_get_val (outputs->pdata[0]) != 9.0
	   && g_get_monotonic_time () < end)
      g_main_context_iteration (NULL, TRUE);
  }
  outputs = st_client_get_outputs (client);
  g_assert_cmpfloat (st_output_get_val (outputs->pdata[0]), ==, 9.0);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListInputs", NULL,
				 G_VARIANT_TYPE ("(a{s(siddddt)})"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  {
    g_autoptr (GVariant) dict = g_variant_get_child_value (reply, 0);
    g_autoptr (GVariant) entry = NULL;
    const gchar *descr = NULL;
    gint unit = 0;
    gdouble min = 0, max = 0, step = 0, value = 0;
    guint64 flags = 0;

    g_assert_cmpuint (g_variant_n_children (dict), ==, 1);
    entry = g_variant_lookup_value (dict, "test:in",
				    G_VARIANT_TYPE ("(siddddt)"));
    g_assert_nonnull (entry);
    g_variant_get (entry, "(&siddddt)", &descr, &unit, &min, &max, &step,
		   &value, &flags);
    g_assert_cmpfloat (value, ==, 7.0);
  }
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListOutputs", NULL,
				 G_VARIANT_TYPE ("(a{s(siddddt)})"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  {
    g_autoptr (GVariant) dict = g_variant_get_child_value (reply, 0);
    g_autoptr (GVariant) entry = NULL;
    const gchar *descr = NULL;
    gint unit = 0;
    gdouble min = 0, max = 0, step = 0, value = 0;
    guint64 flags = 0;

    g_assert_cmpuint (g_variant_n_children (dict), ==, 1);
    entry = g_variant_lookup_value (dict, "test:out",
				    G_VARIANT_TYPE ("(siddddt)"));
    g_assert_nonnull (entry);
    g_variant_get (entry, "(&siddddt)", &descr, &unit, &min, &max, &step,
		   &value, &flags);
    g_assert_cmpfloat (value, ==, 9.0);
  }
  g_variant_unref (reply);

  g_object_unref (conn);

  g_main_loop_quit (peer.loop);
  g_thread_join (peer.thread);
  g_main_loop_unref (peer.loop);
  g_main_context_unref (peer.context);
  g_mutex_clear (&peer.mutex);
  g_cond_clear (&peer.cond);
  g_clear_error (&peer.error);
}

static void
test_debug_server_list_io_empty (Fixture * f, gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListInputs", NULL,
				 G_VARIANT_TYPE ("(a{s(siddddt)})"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  {
    g_autoptr (GVariant) dict = g_variant_get_child_value (reply, 0);

    g_assert_cmpuint (g_variant_n_children (dict), ==, 0);
  }
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListOutputs", NULL,
				 G_VARIANT_TYPE ("(a{s(siddddt)})"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  {
    g_autoptr (GVariant) dict = g_variant_get_child_value (reply, 0);

    g_assert_cmpuint (g_variant_n_children (dict), ==, 0);
  }
  g_variant_unref (reply);

  g_object_unref (conn);
}

static gboolean
stopped_seen (gpointer data)
{
  return ((Fixture *) data)->peer_signals > 0;	/* reused counter below */
}

static void
on_stopped_signal (GDBusConnection * connection,
		   const gchar * sender_name,
		   const gchar * object_path,
		   const gchar * interface_name,
		   const gchar * signal_name, GVariant * parameters,
		   gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *reason = NULL;
  const gchar *id = NULL;
  gdouble value = 0;
  guint handle = 0;

  (void) connection;
  (void) sender_name;
  (void) object_path;
  (void) interface_name;
  (void) signal_name;

  g_variant_get (parameters, "(&s&sdu)", &reason, &id, &value, &handle);
  f->peer_signals++;		/* reuse as stopped count */
  g_free (f->last_peer_event);
  g_free (f->last_peer_name);
  f->last_peer_event = g_strdup (reason);
  f->last_peer_name = g_strdup (id);
  f->last_input_val = value;
  f->last_peer_port = handle;
}

typedef struct
{
  Fixture *f;
  gboolean done;
  gboolean hit;
  const gchar *id;
  gdouble value;
  gboolean use_watch;
} HitJob;

static gboolean
hit_on_server (gpointer data)
{
  HitJob *job = data;

  if (job->use_watch)
    job->hit =
      st_core_debug_intercept_change (job->f->debug, job->id, job->value);
  else
    job->hit = st_core_debug_intercept_apply (job->f->debug, job->id);
  job->done = TRUE;
  return G_SOURCE_REMOVE;
}

static void
test_debug_server_breakpoints_watchpoints (Fixture * f,
					   gconstpointer user_data)
{
  GError *error = NULL;
  GDBusConnection *conn;
  GVariant *reply;
  guint bp = 0, wp = 0;
  gboolean ok = FALSE;
  gboolean paused = FALSE;
  guint sid;
  HitJob job = { 0 };

  (void) user_data;

  conn = connect_debug (f, &error);
  g_assert_no_error (error);
  g_assert_nonnull (conn);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "AddBreakpoint",
				 g_variant_new ("(s)", "beep:status"),
				 G_VARIANT_TYPE ("(u)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(u)", &bp);
  g_assert_cmpuint (bp, >, 0);
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "AddWatchpoint",
				 g_variant_new ("(s)", "time:second"),
				 G_VARIANT_TYPE ("(u)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(u)", &wp);
  g_assert_cmpuint (wp, >, 0);
  g_assert_cmpuint (wp, !=, bp);
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "ListBreakpoints", NULL,
				 G_VARIANT_TYPE ("(a(us))"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpuint (g_variant_n_children
		    (g_variant_get_child_value (reply, 0)), ==, 1);
  g_variant_unref (reply);

  sid =
    g_dbus_connection_signal_subscribe (conn, NULL, DEBUG_INTERFACE,
					"Stopped", DEBUG_OBJECT_PATH, NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					on_stopped_signal, f, NULL);
  g_assert_cmpuint (sid, !=, 0);

  f->peer_signals = 0;
  job.f = f;
  job.id = "time:second";
  job.value = 42;
  job.use_watch = TRUE;
  g_main_context_invoke (f->context, hit_on_server, &job);
  wait_server_job (f, (ServerEmitJob *) & job);
  g_assert_true (job.hit);
  g_assert_true (st_core_debug_is_paused (f->debug));
  g_assert_true (iterate_until (stopped_seen, f, 2000));
  g_assert_cmpstr (f->last_peer_event, ==, "watchpoint");
  g_assert_cmpstr (f->last_peer_name, ==, "time:second");
  g_assert_cmpfloat (f->last_input_val, ==, 42.0);

  /* While paused, further changes stay intercepted. */
  g_assert_true (st_core_debug_intercept_change
		 (f->debug, "other:id", 1.0));

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "Continue", NULL, NULL,
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_clear_pointer (&reply, g_variant_unref);
  g_assert_false (st_core_debug_is_paused (f->debug));

  f->peer_signals = 0;
  job.done = FALSE;
  job.hit = FALSE;
  job.id = "beep:status";
  job.value = 0;
  job.use_watch = FALSE;
  g_main_context_invoke (f->context, hit_on_server, &job);
  wait_server_job (f, (ServerEmitJob *) & job);
  g_assert_true (job.hit);
  g_assert_true (iterate_until (stopped_seen, f, 2000));
  g_assert_cmpstr (f->last_peer_event, ==, "breakpoint");
  g_assert_cmpstr (f->last_peer_name, ==, "beep:status");

  g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
			       DEBUG_INTERFACE, "Continue", NULL, NULL,
			       G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "RemoveWatchpoint",
				 g_variant_new ("(u)", wp),
				 G_VARIANT_TYPE ("(b)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(b)", &ok);
  g_assert_true (ok);
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "RemoveBreakpoint",
				 g_variant_new ("(u)", bp),
				 G_VARIANT_TYPE ("(b)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(b)", &ok);
  g_assert_true (ok);
  g_variant_unref (reply);

  reply =
    g_dbus_connection_call_sync (conn, NULL, DEBUG_OBJECT_PATH,
				 DEBUG_INTERFACE, "GetPaused", NULL,
				 G_VARIANT_TYPE ("(b)"),
				 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error (error);
  g_variant_get (reply, "(b)", &paused);
  g_assert_false (paused);
  g_variant_unref (reply);

  g_dbus_connection_signal_unsubscribe (conn, sid);
  g_object_unref (conn);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/starter-core/debug-server/bind-parse-errors",
		   test_debug_server_bind_parse_errors);

  g_test_add ("/starter-core/debug-server/start-stop", Fixture, NULL,
	      fixture_setup, test_debug_server_start_stop, fixture_teardown);

  g_test_add ("/starter-core/debug-server/get-status", Fixture, NULL,
	      fixture_setup, test_debug_server_get_status, fixture_teardown);

  g_test_add ("/starter-core/debug-server/list-peers", Fixture, NULL,
	      fixture_setup, test_debug_server_list_peers, fixture_teardown);

  g_test_add ("/starter-core/debug-server/peer-signal", Fixture, NULL,
	      fixture_setup, test_debug_server_peer_signal, fixture_teardown);

  g_test_add ("/starter-core/debug-server/input-signal", Fixture, NULL,
	      fixture_setup, test_debug_server_input_signal,
	      fixture_teardown);

  g_test_add ("/starter-core/debug-server/rule-trigger-signals", Fixture,
	      NULL, fixture_setup, test_debug_server_rule_trigger_signals,
	      fixture_teardown);

  g_test_add ("/starter-core/debug-server/set-unknown", Fixture, NULL,
	      fixture_setup, test_debug_server_set_unknown, fixture_teardown);

  g_test_add ("/starter-core/debug-server/list-io-empty", Fixture, NULL,
	      fixture_setup, test_debug_server_list_io_empty, fixture_teardown);

  g_test_add ("/starter-core/debug-server/set-input-output", Fixture, NULL,
	      fixture_setup, test_debug_server_set_input_output,
	      fixture_teardown);

  g_test_add ("/starter-core/debug-server/breakpoints-watchpoints",
	      Fixture, NULL, fixture_setup,
	      test_debug_server_breakpoints_watchpoints, fixture_teardown);

  return g_test_run ();
}
