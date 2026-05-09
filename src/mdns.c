/*
 * starter-core - mdns.c
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

#include <stdio.h>

#include <gio/gio.h>

#include "internals.h"

static void
on_mdns_service_removed (STMDNS * mdns, const gchar * name,
			 gpointer user_data)
{
  STCore *self = user_data;

  if (g_hash_table_contains (self->clients, name))
    {
      LOGD ("remove client with name: %s", name);

      g_hash_table_remove (self->clients, name);

      g_hash_table_unref (self->all_inputs);
      self->all_inputs = st_core_setup_inputs_hash_table (self);

      g_hash_table_unref (self->all_outputs);
      self->all_outputs = st_core_setup_outputs_hash_table (self);
    }
}

static void
on_mdns_service_added (STMDNS * mdns, STMDNSService * service,
		       gpointer user_data)
{
  STCore *self = user_data;
  gchar *name;
  gchar *type;
  gint proto;

  name = st_mdns_service_dup_name (service);
  type = st_mdns_service_dup_mdns_type (service);
  proto = st_mdns_service_get_proto (service);

  LOGD ("service added: %s:%s:%d", name, type, proto);

  if (g_regex_match_simple (self->srvmatch, name, G_REGEX_OPTIMIZE, 0)
      && !g_hash_table_contains (self->clients, name))
    {
      STClient *client;
      GError *error = NULL;

      client = st_client_new ();

      if (st_client_start
	  (client, st_mdns_service_get_address (service),
	   st_mdns_service_get_port (service), &error))
	{
	  const GPtrArray *inputs;
	  const GPtrArray *outputs;
	  const STStats *stats;
	  guint i;

	  LOGD ("Client created and started successfully, add to map");

	  g_hash_table_insert (self->clients, g_strdup (name), client);

	  inputs = st_client_get_inputs (client);

	  for (i = 0; i < inputs->len; i++)
	    {
	      const STInput *in = inputs->pdata[i];
	      guint64 flags;

	      LOGD ("connect changed signal from Input %s",
		    st_input_get_name (in));

	      g_signal_connect ((STInput *) in, "val-changed",
				G_CALLBACK (st_core_input_changed_callback),
				self);

	      flags = st_input_get_flags (in);

	      if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
		{
		  gdouble db_val;

		  st_mysql_client_store_input_label (self->mysql, in, NULL);

		  db_val =
		    st_mysql_client_get_last_history_value (self->mysql,
							    st_input_get_name
							    (in), NULL);

		  if (db_val != st_input_get_val (in))
		    {
		      GError *error = NULL;

		      if (!st_mysql_client_store_history
			  (self->mysql, st_input_get_name (in),
			   st_input_get_val (in), st_input_get_flags (in),
			   &error))
			{
			  LOGE ("error insert history: %s",
				error ? error->message : "unknown error");
			  g_error_free (error);
			}
		    }
		}
	    }

	  outputs = st_client_get_outputs (client);

	  for (i = 0; i < outputs->len; i++)
	    {
	      const STOutput *out = outputs->pdata[i];
	      guint64 flags;

	      LOGD ("connect changed signal from Output %s",
		    st_output_get_name (out));

	      g_signal_connect ((STOutput *) out, "val-changed",
				G_CALLBACK (st_core_output_changed_callback),
				self);

	      flags = st_output_get_flags (out);

	      if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
		{
		  gdouble db_val;

		  st_mysql_client_store_output_label (self->mysql, out, NULL);

		  db_val =
		    st_mysql_client_get_last_history_value (self->mysql,
							    st_output_get_name
							    (out), NULL);

		  if (db_val != st_output_get_val (out))
		    {
		      GError *error = NULL;

		      if (!st_mysql_client_store_history
			  (self->mysql, st_output_get_name (out),
			   st_output_get_val (out), st_output_get_flags (out),
			   &error))
			{
			  LOGE ("error insert history: %s",
				error ? error->message : "unknown error");
			  g_error_free (error);
			}
		    }
		}
	    }

	  stats = st_client_get_stats (client);

	  g_signal_connect ((STStats *) stats, "changed",
			    G_CALLBACK (st_core_stats_changed_callback),
			    self);

	  g_hash_table_unref (self->all_inputs);
	  self->all_inputs = st_core_setup_inputs_hash_table (self);

	  g_hash_table_unref (self->all_outputs);
	  self->all_outputs = st_core_setup_outputs_hash_table (self);

	  if (1)
	    {
	      GError *error = NULL;

	      if (!engine_apply
		  (self->engine, self->all_inputs, self->all_outputs, NULL,
		   &error))
		{
		  LOGW ("error from engine_apply: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		}
	    }
	}
      else
	{
	  LOGE ("error starting client for service %s: %s", name,
		error ? error->message : "unknown error");

	  g_object_unref (client);
	}

      if (error)
	g_error_free (error);
    }

  g_free (name);
  g_free (type);
}

gboolean
st_core_mdns_init (STCore * self)
{
  gboolean r = FALSE;

  if (DISCOVERY_ENABLED (self))
    {
      if ((self->mdns = st_mdns_new ()) != NULL)
	{
	  g_signal_connect (self->mdns, "service-added",
			    G_CALLBACK (on_mdns_service_added), self);
	  g_signal_connect (self->mdns, "service-removed",
			    G_CALLBACK (on_mdns_service_removed), self);

	  r = TRUE;
	}
    }
  else
    r = TRUE;

  return r;
}

void
st_core_mdns_finish (STCore * self)
{
  if (DISCOVERY_ENABLED (self) && self->mdns)
    g_object_unref (G_OBJECT (self->mdns));
}
