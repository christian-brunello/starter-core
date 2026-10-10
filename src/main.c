/*
 * starter-core - main.c
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
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <gio/gio.h>

#include <starter/input.h>
#include <starter/output.h>
#include <starter/client.h>
#include <starter/mdns.h>

#include "internals.h"

#ifdef ENABLE_DEBUG_SERVER
#include "debug-server.h"
#endif

VerboseLevel verbose_level = VERBOSE_LEVEL_1;

static int version_flag = 0;

extern int yyparse (Engine * engine);

static const gchar *
st_core_log_domain_from_fields (const GLogField * fields, gsize n_fields)
{
  gsize i;

  for (i = 0; i < n_fields; i++)
    if (g_strcmp0 (fields[i].key, "GLIB_DOMAIN") == 0)
      return fields[i].value;

  return NULL;
}

/*
 * GLib's default formatter prefixes WARNING/CRITICAL with a blank line
 * ("\n(prog:pid): ..."). Strip that for denser journal/console output.
 * Keep GLib's INFO/DEBUG gating via G_MESSAGES_DEBUG.
 */
static GLogWriterOutput
st_core_log_writer (GLogLevelFlags log_level,
		    const GLogField * fields,
		    gsize n_fields, gpointer user_data)
{
  g_autofree gchar *formatted = NULL;
  const gchar *domain;
  const gchar *p;

  domain = st_core_log_domain_from_fields (fields, n_fields);
  if (g_log_writer_default_would_drop (log_level, domain))
    return G_LOG_WRITER_HANDLED;

  if (g_log_writer_is_journald (fileno (stderr)))
    return g_log_writer_journald (log_level, fields, n_fields, user_data);

  formatted = g_log_writer_format_fields (log_level, fields, n_fields, FALSE);
  if (formatted == NULL)
    return G_LOG_WRITER_UNHANDLED;

  p = formatted;
  while (*p == '\n')
    p++;

  fputs (p, stderr);
  fputc ('\n', stderr);

  return G_LOG_WRITER_HANDLED;
}

static gboolean
load_settings (STCore * self)
{
  self->settings = g_settings_new ("org.starter.core");

  if (self->settings != NULL)
    {
      self->rules_preprocessor =
	g_settings_get_string (self->settings, "rules-preprocessor");
    }
  else
    LOGE ("error load settings");

  return self->settings != NULL;
}

static void
print_version (void)
{
  printf ("%s %s\n", PACKAGE, VERSION);
  printf ("Written by Christian Brunello\n\n");
  printf ("Copyright (C) 2026 Christian Brunello.\n");
  printf
    ("This is free software; see the source for copying conditions.  There is NO\n"
     "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
}

static gboolean
parse_command_line (STCore * self, int *argc, char ***argv)
{
  gboolean r = FALSE;

  GOptionEntry entries[] = {
    {"service-match", 'm', 0, G_OPTION_ARG_STRING, &self->srvmatch,
     "regular expression to match services (default: '^(STARTER:|ST:).*')",
     NULL},
    {"no-discovery", 'n', 0, G_OPTION_ARG_NONE, &self->no_discovery,
     "Do not discovery devices", NULL},
    {"rules", 'R', 0, G_OPTION_ARG_STRING, &self->rules_file,
     "Path to rules file", NULL},
    {"rules-preprocessor", 'P', 0, G_OPTION_ARG_STRING,
     &self->rules_preprocessor, "Command to preprocess rules-file", NULL},
    {"verbosity", 'V', 0, G_OPTION_ARG_INT, &verbose_level,
     "stderr verbosity: 0=off 1=info 2=info+debug 3=+describe (default: 1)",
     NULL},
    {"version", 'v', 0, G_OPTION_ARG_NONE, &version_flag,
     "Print version and exit", NULL},
#ifdef ENABLE_DEBUG_SERVER
    {"debug-server-bind", 0, 0, G_OPTION_ARG_STRING, &self->debug_server_bind,
     "Debug server listen address (default: " ST_CORE_DEBUG_DEFAULT_BIND ")",
     "HOST:PORT"},
#endif
    {0}
  };

  GOptionContext *context;
  GError *error = NULL;

  context =
    g_option_context_new ("STARTER (Simple distribuTed smARt conTrollER)");
  g_option_context_add_main_entries (context, entries, NULL);

  if (g_option_context_parse (context, argc, argv, &error))
    {
      r = TRUE;
    }
  else
    {
      LOGE ("Error parsing command line arguments: %s", error->message);
      g_error_free (error);
    }

  g_option_context_free (context);

  if (self->srvmatch == NULL)
    self->srvmatch = g_strdup ("^(STARTER:|ST:).*");

  return r;
}

void
st_core_var_changed_callback (STVar * var, gpointer user_data)
{
  STCore *self = user_data;
  GError *error = NULL;
  const gchar *name = st_var_get_name (var);

  if (ST_LOG_DEBUG_ENABLED ())
    {
      g_autoptr (GString) s = g_string_new ("");

      LOGD ("variable changed: %s = %s", name,
	    expr_describe (st_var_get_value (var), s));
    }
  else
    LOGD ("variable changed: %s", name);

  if (verbose_level >= VERBOSE_LEVEL_2)
    {
      g_autoptr (GString) s = g_string_new ("");

      VERBOSE_2_PRINTF ("Variable changed: %s -> %s", name,
			expr_describe (st_var_get_value (var), s));
    }

#ifdef ENABLE_DEBUG_SERVER
  if (self->debug)
    {
      gdouble v = 0;

      st_core_debug_emit_var_changed (self->debug, var);
      expr_eval (st_var_get_value (var), self->all_inputs, self->all_outputs,
		 self->engine->variables, &v, NULL);
      if (st_core_debug_intercept_change (self->debug, name, v))
	return;
    }
#endif

  if (!engine_apply
      (self->engine, self->all_inputs, self->all_outputs, name, &error))
    {
      LOGW ("error from engine_apply: %s",
	    error ? error->message : "unknown error");
      g_error_free (error);
    }
}

static void
foreach_connect_changed_signal (gpointer key, gpointer value,
				gpointer user_data)
{
  g_signal_connect ((STVar *) value, "changed",
		    G_CALLBACK (st_core_var_changed_callback), user_data);
}

static gboolean
parse_rules_file (STCore * self)
{
  gboolean r = FALSE;

  LOGD ("parse rules file; preprocessor: %s, rules_file: %s",
	self->rules_preprocessor, self->rules_file);

  if (self->rules_preprocessor != NULL && *self->rules_preprocessor != 0)
    {
      GString *cmd;
      gint spawn_argc = 0;
      gchar **spawn_argv = NULL;
      gchar *stdout_buf = NULL;
      gchar *stderr_buf = NULL;
      gint exit_status = 0;
      GError *error = NULL;
      gboolean success;

      cmd = g_string_new (self->rules_preprocessor);

      g_string_replace (cmd, "{F}", self->rules_file, 0);

      LOGD ("preprocessor command results in: '%s'", cmd->str);

      if (g_shell_parse_argv (cmd->str, &spawn_argc, &spawn_argv, &error))
	{
	  if (g_spawn_sync (NULL,
			    spawn_argv,
			    NULL,
			    G_SPAWN_SEARCH_PATH,
			    NULL, NULL,
			    &stdout_buf, &stderr_buf, &exit_status, &error))
	    {
	      if (exit_status == 0)
		{
		  if (stdout_buf)
		    {
		      extern FILE *yyin;
		      size_t stdout_buf_len;

		      stdout_buf_len = strlen (stdout_buf);
		      yyin =
			fmemopen ((void *) stdout_buf, stdout_buf_len, "r");

		      if (yyparse (self->engine) == 0)
			{
			  g_hash_table_foreach (self->engine->variables,
						foreach_connect_changed_signal,
						self);

			  LOGI ("rules loaded from preprocessor (%s)",
				self->rules_file);
			  r = TRUE;
			}
		    }
		}
	      else
		LOGE ("bad exit status from preprocessor command: %d",
		      exit_status);

	      g_free (stdout_buf);
	      g_free (stderr_buf);
	    }
	  else
	    {
	      LOGE ("error swawn preprocessor: %s",
		    error ? error->message : "unknown error");
	    }

	  g_string_free (cmd, TRUE);
	  g_strfreev (spawn_argv);

	}
      else
	{
	  LOGE ("error parsing 'rules-preprocessor' command: %s",
		error ? error->message : "unknown error");
	}

      if (error)
	g_error_free (error);
    }

  return r;
}

void
st_core_input_changed_callback (STInput * in, gpointer user_data)
{
  STCore *self = user_data;
  const gchar *name = st_input_get_name (in);
  gdouble val = st_input_get_val (in);
  guint64 flags;
  GError *error = NULL;

  LOGD ("input changed: %s = %g flags=%" G_GUINT64_FORMAT, name, val,
	st_input_get_flags (in));

  VERBOSE_2_PRINTF ("Input changed: %s -> %lf", name, val);

#ifdef ENABLE_DEBUG_SERVER
  if (self->debug)
    {
      st_core_debug_emit_input_changed (self->debug, in);
      if (st_core_debug_intercept_change (self->debug, name, val))
	{
	  flags = st_input_get_flags (in);
	  if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
	    {
	      if (!st_mysql_client_store_history
		  (self->mysql, name, val, flags, &error))
		{
		  LOGE ("error insert history: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		}
	    }
	  return;
	}
    }
#endif

  flags = st_input_get_flags (in);

  if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
    {
      if (!st_mysql_client_store_history
	  (self->mysql, name, val, flags, &error))
	{
	  LOGE ("error insert history: %s",
		error ? error->message : "unknown error");
	  g_error_free (error);
	}
    }
  if (!engine_apply
      (self->engine, self->all_inputs, self->all_outputs, name, &error))
    {
      LOGW ("error from engine_apply: %s",
	    error ? error->message : "unknown error");
      g_error_free (error);
    }
}

void
st_core_output_changed_callback (STOutput * out, gpointer user_data)
{
  STCore *self = user_data;
  const gchar *name = st_output_get_name (out);
  gdouble val = st_output_get_val (out);
  guint64 flags;
  GError *error = NULL;

  LOGD ("output changed: %s = %g flags=%" G_GUINT64_FORMAT, name, val,
	st_output_get_flags (out));

  VERBOSE_2_PRINTF ("Output changed: %s -> %lf", name, val);

#ifdef ENABLE_DEBUG_SERVER
  if (self->debug)
    {
      st_core_debug_emit_output_changed (self->debug, out);
      if (st_core_debug_intercept_change (self->debug, name, val))
	{
	  flags = st_output_get_flags (out);
	  if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
	    {
	      if (!st_mysql_client_store_history
		  (self->mysql, name, val, flags, &error))
		{
		  LOGE ("error insert history: %s",
			error ? error->message : "unknown error");
		  g_error_free (error);
		}
	    }
	  return;
	}
    }
#endif

  flags = st_output_get_flags (out);

  if ((flags & ST_IO_FLAG_NO_HISTORY) == 0)
    {
      if (!st_mysql_client_store_history
	  (self->mysql, name, val, flags, &error))
	{
	  LOGE ("error insert history: %s",
		error ? error->message : "unknown error");
	  g_error_free (error);
	}
    }
  if (!engine_apply
      (self->engine, self->all_inputs, self->all_outputs, name, &error))
    {
      LOGW ("error from engine_apply: %s",
	    error ? error->message : "unknown error");
      g_error_free (error);
    }
}

void
st_core_stats_changed_callback (STStats * stats, gpointer user_data)
{
  STCore *self = user_data;
  GError *error = NULL;

  LOGD ("stats changed: %s", st_stats_get_name (stats));

#ifdef ENABLE_DEBUG_SERVER
  if (self->debug)
    st_core_debug_emit_stats_changed (self->debug, stats);
#endif

  if (!st_mysql_client_store_stats (self->mysql, stats, &error))
    {
      LOGE ("error insert stats: %s",
	    error ? error->message : "unknown error");
      g_error_free (error);
    }

}



static void
foreach_add_inputs (gpointer key, gpointer value, gpointer user_data)
{
  STClient *c = value;
  const GPtrArray *inputs;
  guint i;

  inputs = st_client_get_inputs (c);

  for (i = 0; i < inputs->len; i++)
    {
      ClientInputMap *p = g_malloc0 (sizeof (ClientInputMap));
      gchar *name = st_input_dup_name (inputs->pdata[i]);

      p->client = c;
      p->index = i;

      VERBOSE_3_PRINTF ("  %s", name);

      g_hash_table_insert (user_data, name, p);
    }
}

GHashTable *
st_core_setup_inputs_hash_table (STCore * self)
{
  GHashTable *r;

  VERBOSE_2_PRINTF ("Rebuild inputs table");

  r = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_hash_table_foreach (self->clients, foreach_add_inputs, r);

  return r;
}

static void
foreach_add_outputs (gpointer key, gpointer value, gpointer user_data)
{
  STClient *c = value;
  const GPtrArray *outputs;
  guint i;

  outputs = st_client_get_outputs (c);

  for (i = 0; i < outputs->len; i++)
    {
      ClientOutputMap *p = g_malloc0 (sizeof (ClientOutputMap));
      gchar *name = st_output_dup_name (outputs->pdata[i]);

      p->client = c;
      p->index = i;

      VERBOSE_3_PRINTF ("  %s", name);

      g_hash_table_insert (user_data, name, p);
    }
}

GHashTable *
st_core_setup_outputs_hash_table (STCore * self)
{
  GHashTable *r;

  VERBOSE_2_PRINTF ("Rebuild outputs table");

  r = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_hash_table_foreach (self->clients, foreach_add_outputs, r);

  return r;
}

#ifdef ENABLE_DEBUG_SERVER
static void
on_debug_rule_triggered (const gchar * dest, const gchar * describe,
			 gpointer user_data)
{
  STCore *self = user_data;

  st_core_debug_emit_rule_triggered (self->debug, dest, describe);
}

static void
on_debug_trigger_triggered (const gchar * dest, const gchar * describe,
			    gpointer user_data)
{
  STCore *self = user_data;

  st_core_debug_emit_trigger_triggered (self->debug, dest, describe);
}
#endif

int
main (int argc, char *argv[])
{
  STCore self;
  GMainLoop *gloop = NULL;
  gchar *db_host;
  guint db_port;
  gchar *db_user;
  gchar *db_pass;
  guint passlen;

  g_log_set_writer_func (st_core_log_writer, NULL, NULL);

  memset (&self, 0x00, sizeof self);

  self.engine =
    engine_new (g_ptr_array_new_full (0, (GDestroyNotify) lable_block_delete),
		g_ptr_array_new_full (0, (GDestroyNotify) trigger_delete));

  if (!load_settings (&self))
    exit (EXIT_FAILURE);

  if (!parse_command_line (&self, &argc, &argv))
    exit (EXIT_FAILURE);

  if (version_flag)
    {
      print_version ();
      exit (EXIT_SUCCESS);
    }

  if (!parse_rules_file (&self))
    exit (EXIT_FAILURE);

  if (verbose_level >= VERBOSE_LEVEL_3 || ST_LOG_DEBUG_ENABLED ())
    {
      g_autoptr (GString) s = g_string_new ("");

      engine_describe (self.engine, s);
      VERBOSE_3_PRINTF ("ENGINE:\n%s", s->str);
      LOGD ("engine: %s", s->str);
    }

  self.self_stats = st_stats_new ("ST:Core");
  self.self_coll =
    ST_STATS_COLLECTOR (st_process_stats_collector_new
			(self.self_stats, 1000));

  g_signal_connect (self.self_stats, "changed",
		    G_CALLBACK (st_core_stats_changed_callback), &self);

  self.clients =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self.pending_clients = NULL;
  self.all_inputs = st_core_setup_inputs_hash_table (&self);
  self.all_outputs = st_core_setup_outputs_hash_table (&self);

  db_host = g_settings_get_string (self.settings, "db-host");
  db_port = g_settings_get_int (self.settings, "db-port");
  db_user = g_settings_get_string (self.settings, "db-user");
  db_pass = g_settings_get_string (self.settings, "db-pass");
  passlen = strlen (db_pass);

  self.mysql =
    st_mysql_client_new ("STCore", db_host, db_port, db_user, db_pass);

  g_free (db_host);
  g_free (db_user);
  memset (db_pass, 0x00, passlen);	// do not keep in memory
  g_free (db_pass);

  gloop = g_main_loop_new (NULL, FALSE);

#ifdef ENABLE_DEBUG_SERVER
  {
    GError *debug_error = NULL;

    self.debug = st_core_debug_new (&self);
    if (!st_core_debug_start (self.debug, self.debug_server_bind,
			      &debug_error))
      {
	LOGE ("failed to start debug server: %s",
	      debug_error ? debug_error->message : "unknown");
	g_clear_error (&debug_error);
	exit (EXIT_FAILURE);
      }

    self.engine->on_rule_triggered = on_debug_rule_triggered;
    self.engine->on_rule_triggered_data = &self;
    self.engine->on_trigger_triggered = on_debug_trigger_triggered;
    self.engine->on_trigger_triggered_data = &self;
  }
#endif

  if (!st_core_mdns_init (&self))
    exit (EXIT_FAILURE);

  g_main_loop_run (gloop);

  st_core_mdns_finish (&self);

#ifdef ENABLE_DEBUG_SERVER
  if (self.debug)
    {
      st_core_debug_stop (self.debug);
      g_clear_object (&self.debug);
    }
#endif

  if (self.pending_clients)
    g_hash_table_unref (self.pending_clients);

  g_hash_table_unref (self.clients);

  g_main_loop_unref (gloop);

  return EXIT_SUCCESS;
}
