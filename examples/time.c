/*
 * starter-core - time.c
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
#include <time.h>

#include <glib.h>
#include <gio/gio.h>

#include <starter/mdns.h>
#include <starter/server.h>
#include <starter/input.h>
#include <starter/output.h>

#define ST_TIME_SERVICE_LOG_DOMAIN "starter-time-example"

#define LOGD(...) g_log (ST_TIME_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) g_log (ST_TIME_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) g_log (ST_TIME_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE(...) g_log (ST_TIME_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

enum
{
  IN_HOUR,
  IN_MINUTE,
  IN_SECOND
};

enum
{
  OUT_LOCAL
};

static GPtrArray *inputs = NULL;
static GPtrArray *outputs = NULL;
static gboolean local = FALSE;
static STVersion version = { 0, 0, 1 };

static void
set_input_val (STInput * in, gdouble val)
{
  GError *error = NULL;

  st_input_set_val (in, val, &error);

  if (error)
    {
      g_print ("Error set input %s: %s\n", st_input_get_name (in),
	       error->message);
      g_error_free (error);
    }
}

static void
on_output_changed (STOutput * out, gpointer user_data)
{
  LOGD ("output %s changed to %lf", st_output_get_name (out),
	st_output_get_val (out));

  if (out == outputs->pdata[OUT_LOCAL])
    local = st_output_get_val (out) ? TRUE : FALSE;
  else
    g_print ("unmanaged output %s in %s\n", __FUNCTION__,
	     st_output_get_name (out));
}

static gint
tick (gpointer user_data)
{
  GDateTime *now = NULL;
  gint hour;
  gint minute;
  gint second;
  GError *error = NULL;

  now = local ? g_date_time_new_now_local () : g_date_time_new_now_utc ();

  hour = g_date_time_get_hour (now);
  minute = g_date_time_get_minute (now);
  second = g_date_time_get_second (now);

  LOGD ("apply time %02d:%02d:%2d", hour, minute, second);

  set_input_val (inputs->pdata[IN_HOUR], hour);
  set_input_val (inputs->pdata[IN_MINUTE], minute);
  set_input_val (inputs->pdata[IN_SECOND], second);

  g_date_time_unref (now);

  return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
  GMainLoop *gloop = NULL;
  STServer *server;
  STMDNS *mdns;
  GError *error = NULL;

  srand (time (NULL));

  gloop = g_main_loop_new (NULL, FALSE);

  // use ST_IO_FLAG_NO_HISTORY to avoid database storage
  inputs = g_ptr_array_new_full (0, g_object_unref);
  g_ptr_array_add (inputs,
		   st_input_new ("time:hour", "Current hour", ST_UNIT_NULL, 0,
				 23, 1, 0, ST_IO_FLAG_NO_HISTORY));
  g_ptr_array_add (inputs,
		   st_input_new ("time:minute", "Current minute",
				 ST_UNIT_NULL, 0, 59, 1, 0,
				 ST_IO_FLAG_NO_HISTORY));
  g_ptr_array_add (inputs,
		   st_input_new ("time:second", "Current second",
				 ST_UNIT_NULL, 0, 59, 1, 0,
				 ST_IO_FLAG_NO_HISTORY));

  outputs = g_ptr_array_new_full (0, g_object_unref);
  g_ptr_array_add (outputs,
		   st_output_new ("time:local", "use local time",
				  ST_UNIT_ONOFF, 0, 1, 1, 0,
				  ST_IO_FLAG_NO_HISTORY));
  g_signal_connect (outputs->pdata[OUT_LOCAL], "val-changed",
		    G_CALLBACK (on_output_changed), NULL);

  server = st_server_new ("ST:TimeExample", &version, inputs, outputs);

  // Port 0 to make dynamic allocation
  st_server_start (server, 0, &error);

  mdns = st_mdns_new ();

  st_mdns_publish_service (mdns, st_server_get_name (server), "_dbus._tcp",
			   st_server_get_port (server));

  g_timeout_add_seconds (1, tick, NULL);

  g_main_loop_run (gloop);

  return EXIT_SUCCESS;
}
