/*
 * libstarter-core - client.c
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

#include <glib.h>
#include <gio/gio.h>

#include <starter/error.h>
#include <starter/stats.h>
#include <starter/client.h>

#include <starter/input.h>
#include <starter/output.h>

#include "internals.h"

typedef struct
{
  GDBusConnection *connection;
  guint input_changed_sid;
  guint output_changed_sid;
  guint stats_changed_sid;
  GPtrArray *inputs;
  GPtrArray *outputs;
  STStats *stats;
  STClientState state;
  GCancellable *start_cancellable;
} STClientPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (STClient, st_client, G_TYPE_OBJECT)
#define ST_CLIENT_GET_PRIVATE(obj) \
    ((STClientPrivate *) st_client_get_instance_private (ST_CLIENT (obj)))

static void handle_input_changed_signal (GDBusConnection * connection, const gchar * sender_name, const gchar * object_path, const gchar * interface_name, const gchar * signal_name, GVariant * parameters,	// Signal arguments
					      gpointer user_data)
{
  STClient *self = user_data;
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("received input changed signal");

  GVariant *key_value_pair;

  g_variant_get (parameters, "(@{s(siddddt)})", &key_value_pair);

  if (key_value_pair != NULL)
    {
      gchar *name;
      GVariant *tuple_variant;
      gchar *descr;
      gint unit;
      gdouble min, max, step, val;
      guint64 flags;
      guint i;

      g_variant_get (key_value_pair, "{s@(siddddt)}", &name, &tuple_variant);
      g_variant_get (tuple_variant, "(siddddt)", &descr, &unit, &min, &max,
		     &step, &val, &flags);

      g_variant_unref (tuple_variant);
      g_variant_unref (key_value_pair);

      LOGD
	("Input: %s, descr: %s, unit: %d, min: %lf, max: %lf, step: %lf, val: %lf, flags: %"
	 PRIu64, name, descr, unit, min, max, step, val, flags);

      for (i = 0; i < priv->inputs->len; i++)
	{
	  STInput *in = priv->inputs->pdata[i];

	  if (g_strcmp0 (name, st_input_get_name (in)) == 0)
	    {
	      GError *error = NULL;

	      LOGD ("found match on input at offset %u", i);

	      if (!st_input_set_description (in, descr, &error))
		{
		  LOGE ("error set input description: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_unit (in, unit, &error))
		{
		  LOGE ("error set input unit: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_min (in, min, &error))
		{
		  LOGE ("error set input min: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_max (in, max, &error))
		{
		  LOGE ("error set input max: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_step (in, step, &error))
		{
		  LOGE ("error set input step: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_val (in, val, &error))
		{
		  LOGE ("error set input val: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_input_set_flags (in, flags, &error))
		{
		  LOGE ("error set input flags: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      break;
	    }
	}

      g_free(name);
      g_free(descr);
    }
  else
    LOGE ("error decode signal parameters");
}

static void
handle_output_changed_signal (GDBusConnection * connection, const gchar * sender_name, const gchar * object_path, const gchar * interface_name, const gchar * signal_name, GVariant * parameters,	// Signal arguments
			      gpointer user_data)
{
  STClient *self = user_data;
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("received output changed signal");

  GVariant *key_value_pair;

  g_variant_get (parameters, "(@{s(siddddt)})", &key_value_pair);

  if (key_value_pair != NULL)
    {
      gchar *name;
      GVariant *tuple_variant;
      gchar *descr;
      gint unit;
      gdouble min, max, step, val;
      guint64 flags;
      guint i;

      g_variant_get (key_value_pair, "{s@(siddddt)}", &name, &tuple_variant);
      g_variant_get (tuple_variant, "(siddddt)", &descr, &unit, &min, &max,
		     &step, &val, &flags);

      g_variant_unref (tuple_variant);
      g_variant_unref (key_value_pair);

      LOGD
	("Output: %s, descr: %s, unit: %d, min: %lf, max: %lf, step: %lf, val: %lf, flags: %"
	 PRIu64, name, descr, unit, min, max, step, val, flags);

      for (i = 0; i < priv->outputs->len; i++)
	{
	  STOutput *out = priv->outputs->pdata[i];

	  if (g_strcmp0 (name, st_output_get_name (out)) == 0)
	    {
	      GError *error = NULL;

	      LOGD ("found match on output at offset %u", i);

	      if (!st_output_set_description (out, descr, &error))
		{
		  LOGE ("error set output description: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_unit (out, unit, &error))
		{
		  LOGE ("error set output unit: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_min (out, min, &error))
		{
		  LOGE ("error set output min: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_max (out, max, &error))
		{
		  LOGE ("error set output max: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_step (out, step, &error))
		{
		  LOGE ("error set output step: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_val (out, val, &error))
		{
		  LOGE ("error set output val: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      if (!st_output_set_flags (out, flags, &error))
		{
		  LOGE ("error set output flags: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		  error = NULL;
		}

	      break;
	    }
	}

      g_free(name);
      g_free(descr);
    }
  else
    LOGE ("error decode signal parameters");
}

static void
handle_stats_changed_signal (GDBusConnection * connection, const gchar * sender_name, const gchar * object_path, const gchar * interface_name, const gchar * signal_name, GVariant * parameters,	// Signal arguments
			     gpointer user_data)
{
  STClient *self = user_data;
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  gchar *name;
  STStatsEntry vmsize;
  STStatsEntry vmrss;
  STStatsEntry vmswap;
  STStatsEntry io_read;
  STStatsEntry io_write;
  STStatsEntry priority;
  STStatsEntry threads;
  STStatsEntry icswitch;
  STStatsEntry cpu;

  LOGD ("%s: received stats changed signal", __FUNCTION__);

  g_variant_get (parameters,
		 "((sddddddddddddddddddddddddddd))",
		 &name,
		 &vmsize.min, &vmsize.max, &vmsize.last,
		 &vmrss.min, &vmrss.max, &vmrss.last,
		 &vmswap.min, &vmswap.max, &vmswap.last,
		 &io_read.min, &io_read.max, &io_read.last,
		 &io_write.min, &io_write.max, &io_write.last,
		 &priority.min, &priority.max, &priority.last,
		 &threads.min, &threads.max, &threads.last,
		 &icswitch.min, &icswitch.max, &icswitch.last,
		 &cpu.min, &cpu.max, &cpu.last);

  LOGD ("STStats: {"
	"name: %s "
	"vmsize: %lf,%lf,%lf "
	"vmrss: %lf,%lf,%lf "
	"vmswap: %lf,%lf,%lf "
	"io_read: %lf,%lf,%lf "
	"io_write: %lf,%lf,%lf "
	"priority: %lf,%lf,%lf "
	"threads: %lf,%lf,%lf "
	"icswitch: %lf,%lf,%lf "
	"cpu: %lf,%lf,%lf}",
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

  st_stats_set_full (priv->stats,
		     name,
		     &vmsize,
		     &vmrss,
		     &vmswap,
		     &io_read,
		     &io_write, &priority, &threads, &icswitch, &cpu);

  g_free (name);
}

static gchar *
call_get_name_method (STClient * self, GError ** error)
{
  gchar *r = NULL;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call get name on connection %p", priv->connection);

  if(priv->connection)
    {
      GVariant *result;

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "GetName",
					    NULL,
					    G_VARIANT_TYPE ("(s)"),
					    G_DBUS_CALL_FLAGS_NONE,
					    -1, NULL, error);

      if (result)
	{
	  const gchar *signature;

	  signature = g_variant_get_type_string (result);

	  if (g_strcmp0 (signature, "(s)") == 0)
	    {
	      g_variant_get (result, "(s)", &r);
	    }
	  else
	    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
			 "bad response signature: %s", signature);

	  g_variant_unref (result);
	}
    }
  else
    {
      g_set_error_literal (error,
                           ST_ERROR,
                           ST_ERROR_NOT_CONNECTED, 
                           "client is not connected to D-Bus server");
    }

  return r;
}

static STVersion *
call_get_version_method (STClient * self, GError ** error)
{
  STVersion *r = NULL;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call get name on connection %p", priv->connection);

  if(priv->connection)
    {
      GVariant *result;

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "GetVersion",
					    NULL,
					    G_VARIANT_TYPE ("(iii)"),
					    G_DBUS_CALL_FLAGS_NONE,
					    -1, NULL, error);

      if (result)
	{
	  const gchar *signature;

	  signature = g_variant_get_type_string (result);

	  if (g_strcmp0 (signature, "(iii)") == 0)
	    {
	      gint major;
	      gint minor;
	      gint micro;

	      g_variant_get (result, "(iii)", &major, &minor, &micro);

	      r = g_malloc0 (sizeof (STVersion));

	      r->major = major;
	      r->minor = minor;
	      r->micro = micro;
	    }
	  else
	    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
			 "bad response signature: %s", signature);

	  g_variant_unref (result);
	}
    }
  else
    {
      g_set_error_literal (error,
                           ST_ERROR,
                           ST_ERROR_NOT_CONNECTED, 
                           "client is not connected to D-Bus server");
    }

  return r;
}

static gboolean
apply_inputs_from_result (STClient * self, GVariant * result, GError ** error)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  const gchar *signature;
  gboolean r = FALSE;

  signature = g_variant_get_type_string (result);

  if (g_strcmp0 (signature, "(a{s(siddddt)})") == 0)
    {
      GVariant *map_variant;
      GVariantIter iter;
      GVariant *key_value_pair;

      g_variant_get (result, "(@a{s(siddddt)})", &map_variant);
      g_variant_iter_init (&iter, map_variant);
      g_ptr_array_steal (priv->inputs, NULL);

      while ((key_value_pair = g_variant_iter_next_value (&iter)))
	{
	  gchar *name;
	  gchar *description;
	  STUnit unit;
	  gdouble min, max, step, val;
	  guint64 flags;

	  g_variant_get (key_value_pair, "{s(siddddt)}", &name,
			 &description, &unit, &min, &max, &step, &val,
			 &flags);

	  LOGD
	    ("found input with: name: %s, description: %s, unit: %d, min: %lf, max: %lf, step: %lf, val: %lf, flags: %"
	     PRIu64, name, description, unit, min, max, step, val, flags);

	  g_ptr_array_add (priv->inputs,
			   st_input_new (name, description, unit, min, max,
					 step, val, flags));

	  g_free (name);
	  g_free (description);
	  g_variant_unref (key_value_pair);
	}

      g_variant_unref (map_variant);
      r = TRUE;
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
		 "bad response signature: %s", signature);

  return r;
}

static gboolean
apply_outputs_from_result (STClient * self, GVariant * result,
			   GError ** error)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  const gchar *signature;
  gboolean r = FALSE;

  signature = g_variant_get_type_string (result);

  if (g_strcmp0 (signature, "(a{s(siddddt)})") == 0)
    {
      GVariant *map_variant;
      GVariantIter iter;
      GVariant *key_value_pair;

      g_variant_get (result, "(@a{s(siddddt)})", &map_variant);
      g_variant_iter_init (&iter, map_variant);
      g_ptr_array_steal (priv->outputs, NULL);

      while ((key_value_pair = g_variant_iter_next_value (&iter)))
	{
	  gchar *name;
	  gchar *description;
	  STUnit unit;
	  gdouble min, max, step, val;
	  guint64 flags;

	  g_variant_get (key_value_pair, "{s(siddddt)}", &name,
			 &description, &unit, &min, &max, &step, &val,
			 &flags);

	  LOGD
	    ("found output with: name: %s, description: %s, unit: %d, min: %lf, max: %lf, step: %lf, val: %lf, flags: %"
	     PRIu64, name, description, unit, min, max, step, val, flags);

	  g_ptr_array_add (priv->outputs,
			   st_output_new (name, description, unit, min, max,
					  step, val, flags));

	  g_free (name);
	  g_free (description);
	  g_variant_unref (key_value_pair);
	}

      g_variant_unref (map_variant);
      r = TRUE;
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
		 "bad response signature: %s", signature);

  return r;
}

static gboolean
apply_stats_from_result (STClient * self, GVariant * result, GError ** error)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  const gchar *signature;
  gboolean r = FALSE;

  signature = g_variant_get_type_string (result);

  if (g_strcmp0 (signature, "((sddddddddddddddddddddddddddd))") == 0)
    {
      gchar *name;
      STStatsEntry vmsize, vmrss, vmswap, io_read, io_write;
      STStatsEntry priority, threads, icswitch, cpu;

      g_variant_get (result,
		     "((sddddddddddddddddddddddddddd))",
		     &name,
		     &vmsize.min, &vmsize.max, &vmsize.last,
		     &vmrss.min, &vmrss.max, &vmrss.last,
		     &vmswap.min, &vmswap.max, &vmswap.last,
		     &io_read.min, &io_read.max, &io_read.last,
		     &io_write.min, &io_write.max, &io_write.last,
		     &priority.min, &priority.max, &priority.last,
		     &threads.min, &threads.max, &threads.last,
		     &icswitch.min, &icswitch.max, &icswitch.last,
		     &cpu.min, &cpu.max, &cpu.last);

      st_stats_set_full (priv->stats, name, &vmsize, &vmrss, &vmswap,
			 &io_read, &io_write, &priority, &threads, &icswitch,
			 &cpu);
      g_free (name);
      r = TRUE;
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
		 "bad response signature: %s", signature);

  return r;
}

static void
subscribe_change_signals (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  priv->input_changed_sid =
    g_dbus_connection_signal_subscribe (priv->connection, NULL,
					"org.starter.Service",
					"InputChanged",
					"/org/starter/service", NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					handle_input_changed_signal, self,
					NULL);

  priv->output_changed_sid =
    g_dbus_connection_signal_subscribe (priv->connection, NULL,
					"org.starter.Service",
					"OutputChanged",
					"/org/starter/service", NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					handle_output_changed_signal, self,
					NULL);

  priv->stats_changed_sid =
    g_dbus_connection_signal_subscribe (priv->connection, NULL,
					"org.starter.Service",
					"StatsChanged",
					"/org/starter/service", NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					handle_stats_changed_signal, self,
					NULL);

  LOGD ("signals subscribed (input=%u output=%u stats=%u)",
	priv->input_changed_sid, priv->output_changed_sid,
	priv->stats_changed_sid);
}

static gboolean
call_get_inputs_method (STClient * self, GError ** error)
{
  gboolean r = FALSE;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call get inputs on connection %p", priv->connection);

  if (priv->connection)
    {
      GVariant *result;

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "GetInputs",
					    NULL,
					    G_VARIANT_TYPE ("(a{s(siddddt)})"),
					    G_DBUS_CALL_FLAGS_NONE,
					    -1, NULL, error);

      if (result)
	{
	  r = apply_inputs_from_result (self, result, error);
	  g_variant_unref (result);
	}
    }
  else
    {
      g_set_error_literal (error,
			   ST_ERROR,
			   ST_ERROR_NOT_CONNECTED,
			   "client is not connected to D-Bus server");
    }

  return r;
}

static gboolean
call_get_outputs_method (STClient * self, GError ** error)
{
  gboolean r = FALSE;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call get outputs on connection %p", priv->connection);

  if (priv->connection)
    {
      GVariant *result;

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "GetOutputs",
					    NULL,
					    G_VARIANT_TYPE ("(a{s(siddddt)})"),
					    G_DBUS_CALL_FLAGS_NONE,
					    -1, NULL, error);

      if (result)
	{
	  r = apply_outputs_from_result (self, result, error);
	  g_variant_unref (result);
	}
    }
  else
    {
      g_set_error_literal (error,
			   ST_ERROR,
			   ST_ERROR_NOT_CONNECTED,
			   "client is not connected to D-Bus server");
    }

  return r;
}

static gboolean
call_get_stats_method (STClient * self, GError ** error)
{
  gboolean r = FALSE;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call get stats on connection %p", priv->connection);

  if (priv->connection)
    {
      GVariant *result;

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "GetStats",
					    NULL,
					    G_VARIANT_TYPE
					    ("((sddddddddddddddddddddddddddd))"),
					    G_DBUS_CALL_FLAGS_NONE, -1, NULL,
					    error);

      if (result)
	{
	  r = apply_stats_from_result (self, result, error);
	  g_variant_unref (result);
	}
    }
  else
    {
      g_set_error_literal (error,
			   ST_ERROR,
			   ST_ERROR_NOT_CONNECTED,
			   "client is not connected to D-Bus server");
    }

  return r;
}

static gboolean
call_set_output_method (STClient * self, const gchar * name, gdouble value,
			guint64 flags, GError ** error)
{
  guint32 r = 0;
  STClientPrivate *priv;

  priv = ST_CLIENT_GET_PRIVATE (self);

  LOGD ("call set output on connection %p", priv->connection);

  if(priv->connection)
    {
      GVariant *params;
      GVariant *result;

      params = g_variant_new ("(sdt)", name, value, flags);

      result = g_dbus_connection_call_sync (priv->connection,
					    NULL,
					    "/org/starter/service",
					    "org.starter.Service",
					    "SetOutput",
					    params,
					    G_VARIANT_TYPE ("(i)"),
					    G_DBUS_CALL_FLAGS_NONE,
					    -1, NULL, error);

      if (result)
	{
	  const gchar *signature;

	  signature = g_variant_get_type_string (result);

	  if (g_strcmp0 (signature, "(i)") == 0)
	    {
	      g_variant_get (result, "(i)", &r);
	    }
	  else
	    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_RESPONSE,
			 "bad response signature: %s", signature);

	  g_variant_unref (result);
	}

    }
  else
    {
      g_set_error_literal (error,
                           ST_ERROR,
                           ST_ERROR_NOT_CONNECTED, 
                           "client is not connected to D-Bus server");
    }

  LOGD ("%s completed with r: %u", __FUNCTION__, r);

  return r;
}

static void
st_client_finalize (GObject * object)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (object);

  LOGD ("finalize STClient %p", object);

  if (priv->start_cancellable)
    {
      g_cancellable_cancel (priv->start_cancellable);
      g_clear_object (&priv->start_cancellable);
    }

  if (priv->connection)
    {
      if (priv->input_changed_sid)
	g_dbus_connection_signal_unsubscribe (priv->connection,
					      priv->input_changed_sid);
      if (priv->output_changed_sid)
	g_dbus_connection_signal_unsubscribe (priv->connection,
					      priv->output_changed_sid);
      if (priv->stats_changed_sid)
	g_dbus_connection_signal_unsubscribe (priv->connection,
					      priv->stats_changed_sid);
    }

  g_clear_object (&priv->connection);
  g_ptr_array_free (priv->inputs, TRUE);
  g_ptr_array_free (priv->outputs, TRUE);
  g_clear_object (&priv->stats);

  G_OBJECT_CLASS (st_client_parent_class)->finalize (object);
}

static void
st_client_class_init (STClientClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = st_client_finalize;
}

static void
st_client_init (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  priv->connection = NULL;
  priv->input_changed_sid = 0;
  priv->output_changed_sid = 0;
  priv->stats_changed_sid = 0;
  priv->inputs = g_ptr_array_new_full (0, g_object_unref);
  priv->outputs = g_ptr_array_new_full (0, g_object_unref);
  priv->stats = st_stats_new ("noname");
  priv->state = ST_CLIENT_STATE_IDLE;
  priv->start_cancellable = NULL;
}

STClient *
st_client_new (void)
{
  STClient *r;

  r = g_object_new (ST_TYPE_CLIENT, NULL);

  LOGD ("Created client %p", r);

  return r;
}

STClientState
st_client_get_state (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  return priv->state;
}

gboolean
st_client_is_ready (STClient * self)
{
  return st_client_get_state (self) == ST_CLIENT_STATE_READY;
}

gboolean
st_client_start (STClient * self, const gchar * address, guint16 port,
		 GError ** error)
{
  gboolean r = FALSE;
  STClientPrivate *priv;
  gchar addr[0x40];

  priv = ST_CLIENT_GET_PRIVATE (self);

  if (priv->state == ST_CLIENT_STATE_PENDING
      || priv->state == ST_CLIENT_STATE_READY)
    {
      g_set_error_literal (error, ST_ERROR, ST_ERROR_SYSTEM,
			   "client start already in progress or completed");
      return FALSE;
    }

  priv->state = ST_CLIENT_STATE_PENDING;

  snprintf (addr, sizeof addr, "tcp:host=%s,port=%hu", address, port);

  priv->connection =
    g_dbus_connection_new_for_address_sync (addr,
					    G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
					    NULL, NULL, error);

  if (priv->connection)
    {
      LOGD ("dbus connection estabilished successfully");

      /* Snapshot first, then subscribe, to avoid dropping early signals. */
      if (call_get_inputs_method (self, error)
	  && call_get_outputs_method (self, error)
	  && call_get_stats_method (self, error))
	{
	  subscribe_change_signals (self);
	  priv->state = ST_CLIENT_STATE_READY;
	  r = TRUE;
	}
      else
	{
	  priv->state = ST_CLIENT_STATE_FAILED;
	  g_clear_object (&priv->connection);
	}
    }
  else
    priv->state = ST_CLIENT_STATE_FAILED;

  return r;
}

static void start_async_call_get_inputs (GTask * task);

static void
start_async_fail (GTask * task, GError * error)
{
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  priv->state = ST_CLIENT_STATE_FAILED;
  g_clear_object (&priv->start_cancellable);
  g_clear_object (&priv->connection);
  g_task_return_error (task, error);
  g_object_unref (task);
}

static void
start_async_complete_ready (GTask * task)
{
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  subscribe_change_signals (self);
  priv->state = ST_CLIENT_STATE_READY;
  g_clear_object (&priv->start_cancellable);
  g_task_return_boolean (task, TRUE);
  g_object_unref (task);
}

static void
on_start_async_get_stats (GObject * source, GAsyncResult * res,
			  gpointer user_data)
{
  GTask *task = user_data;
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  GError *error = NULL;
  GVariant *result;

  result =
    g_dbus_connection_call_finish (G_DBUS_CONNECTION (source), res, &error);
  if (!result)
    {
      start_async_fail (task, error);
      return;
    }

  if (!apply_stats_from_result (self, result, &error))
    {
      g_variant_unref (result);
      start_async_fail (task, error);
      return;
    }

  g_variant_unref (result);

  if (g_task_return_error_if_cancelled (task))
    {
      priv->state = ST_CLIENT_STATE_FAILED;
      g_clear_object (&priv->start_cancellable);
      g_clear_object (&priv->connection);
      g_object_unref (task);
      return;
    }

  start_async_complete_ready (task);
}

static void
on_start_async_get_outputs (GObject * source, GAsyncResult * res,
			    gpointer user_data)
{
  GTask *task = user_data;
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  GError *error = NULL;
  GVariant *result;

  result =
    g_dbus_connection_call_finish (G_DBUS_CONNECTION (source), res, &error);
  if (!result)
    {
      start_async_fail (task, error);
      return;
    }

  if (!apply_outputs_from_result (self, result, &error))
    {
      g_variant_unref (result);
      start_async_fail (task, error);
      return;
    }

  g_variant_unref (result);

  if (g_task_return_error_if_cancelled (task))
    {
      priv->state = ST_CLIENT_STATE_FAILED;
      g_clear_object (&priv->start_cancellable);
      g_clear_object (&priv->connection);
      g_object_unref (task);
      return;
    }

  g_dbus_connection_call (priv->connection,
			  NULL,
			  "/org/starter/service",
			  "org.starter.Service",
			  "GetStats",
			  NULL,
			  G_VARIANT_TYPE
			  ("((sddddddddddddddddddddddddddd))"),
			  G_DBUS_CALL_FLAGS_NONE,
			  -1,
			  g_task_get_cancellable (task),
			  on_start_async_get_stats, task);
}

static void
on_start_async_get_inputs (GObject * source, GAsyncResult * res,
			   gpointer user_data)
{
  GTask *task = user_data;
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  GError *error = NULL;
  GVariant *result;

  result =
    g_dbus_connection_call_finish (G_DBUS_CONNECTION (source), res, &error);
  if (!result)
    {
      start_async_fail (task, error);
      return;
    }

  if (!apply_inputs_from_result (self, result, &error))
    {
      g_variant_unref (result);
      start_async_fail (task, error);
      return;
    }

  g_variant_unref (result);

  if (g_task_return_error_if_cancelled (task))
    {
      priv->state = ST_CLIENT_STATE_FAILED;
      g_clear_object (&priv->start_cancellable);
      g_clear_object (&priv->connection);
      g_object_unref (task);
      return;
    }

  g_dbus_connection_call (priv->connection,
			  NULL,
			  "/org/starter/service",
			  "org.starter.Service",
			  "GetOutputs",
			  NULL,
			  G_VARIANT_TYPE ("(a{s(siddddt)})"),
			  G_DBUS_CALL_FLAGS_NONE,
			  -1,
			  g_task_get_cancellable (task),
			  on_start_async_get_outputs, task);
}

static void
start_async_call_get_inputs (GTask * task)
{
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  g_dbus_connection_call (priv->connection,
			  NULL,
			  "/org/starter/service",
			  "org.starter.Service",
			  "GetInputs",
			  NULL,
			  G_VARIANT_TYPE ("(a{s(siddddt)})"),
			  G_DBUS_CALL_FLAGS_NONE,
			  -1,
			  g_task_get_cancellable (task),
			  on_start_async_get_inputs, task);
}

static void
on_start_async_connected (GObject * source, GAsyncResult * res,
			  gpointer user_data)
{
  GTask *task = user_data;
  STClient *self = g_task_get_source_object (task);
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  GError *error = NULL;

  priv->connection =
    g_dbus_connection_new_for_address_finish (res, &error);
  if (!priv->connection)
    {
      start_async_fail (task, error);
      return;
    }

  LOGD ("async dbus connection estabilished successfully");

  if (g_task_return_error_if_cancelled (task))
    {
      priv->state = ST_CLIENT_STATE_FAILED;
      g_clear_object (&priv->start_cancellable);
      g_clear_object (&priv->connection);
      g_object_unref (task);
      return;
    }

  start_async_call_get_inputs (task);
}

void
st_client_start_async (STClient * self, const gchar * address, guint16 port,
		       GCancellable * cancellable,
		       GAsyncReadyCallback callback, gpointer user_data)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);
  GTask *task;
  gchar addr[0x40];

  if (priv->state == ST_CLIENT_STATE_PENDING
      || priv->state == ST_CLIENT_STATE_READY)
    {
      task = g_task_new (self, cancellable, callback, user_data);
      g_task_set_source_tag (task, st_client_start_async);
      g_task_return_new_error (task, ST_ERROR, ST_ERROR_SYSTEM,
			       "client start already in progress or completed");
      g_object_unref (task);
      return;
    }

  priv->state = ST_CLIENT_STATE_PENDING;
  g_clear_object (&priv->start_cancellable);
  if (cancellable)
    priv->start_cancellable = g_object_ref (cancellable);
  else
    priv->start_cancellable = g_cancellable_new ();

  task = g_task_new (self, priv->start_cancellable, callback, user_data);
  g_task_set_source_tag (task, st_client_start_async);

  snprintf (addr, sizeof addr, "tcp:host=%s,port=%hu", address, port);

  g_dbus_connection_new_for_address (addr,
				     G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
				     NULL,
				     g_task_get_cancellable (task),
				     on_start_async_connected, task);
}

gboolean
st_client_start_finish (STClient * self, GAsyncResult * result,
			GError ** error)
{
  g_return_val_if_fail (g_task_is_valid (result, self), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

const GPtrArray *
st_client_get_inputs (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  return priv->inputs;
}

const GPtrArray *
st_client_get_outputs (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  return priv->outputs;
}

const STStats *
st_client_get_stats (STClient * self)
{
  STClientPrivate *priv = ST_CLIENT_GET_PRIVATE (self);

  return priv->stats;
}

gchar *
st_client_get_service_name (STClient * self)
{
  GError *error = NULL;
  gchar *r = NULL;

  if ((r = call_get_name_method (self, &error)) == NULL)
    {
      // LOGE ("error call get name method: %s", error ? error->message : "unknown error");
      g_error_free (error);
    }

  return r;
}

gboolean
st_client_set_output (STClient * self, const gchar * name, double value,
		      guint64 flags)
{
  gboolean r;
  GError *error = NULL;

  r = call_set_output_method (self, name, value, flags, &error);

  if (error)
    {
      LOGE ("error call get name method: %s",
	    error ? error->message : "unknown error");
      g_error_free (error);
    }

  return r;
}
