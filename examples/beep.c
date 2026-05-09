/*
 * starter-core - beep.c
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

#include <starter/mdns.h>
#include <starter/server.h>
#include <starter/input.h>
#include <starter/output.h>

#define ST_OIL_SERVICE_LOG_DOMAIN "starter-oil-example"

#define LOGD(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

enum
{
  OUT_STATUS
};

static GPtrArray *inputs = NULL;
static GPtrArray *outputs = NULL;
static STVersion version = { 0, 0, 1 };

static gboolean status = FALSE;

static void
on_output_changed (STOutput * out, gpointer user_data)
{
  LOGD ("output %s changed to %lf", st_output_get_name (out),
	st_output_get_val (out));

  if (out == outputs->pdata[OUT_STATUS])
    status = st_output_get_val (out) ? TRUE : FALSE;
  else
    g_print ("unmanaged output %s in %s\n", __FUNCTION__,
	     st_output_get_name (out));
}

static gint
tick (gpointer user_data)
{
  if (status)
    g_beep ();

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

  outputs = g_ptr_array_new_full (0, g_object_unref);
  g_ptr_array_add (outputs,
		   st_output_new ("beep:status", "local computer beep",
				  ST_UNIT_ONOFF, 0, 1, 1, status,
				  ST_IO_FLAG_NO_HISTORY));

  server = st_server_new ("ST:Beep", &version, inputs, outputs);

  // Port 0 to make dynamic allocation
  st_server_start (server, 0, &error);

  mdns = st_mdns_new ();

  st_mdns_publish_service (mdns, st_server_get_name (server), "_dbus._tcp",
			   st_server_get_port (server));

  g_timeout_add (500, tick, NULL);

  g_main_loop_run (gloop);

  return EXIT_SUCCESS;
}
