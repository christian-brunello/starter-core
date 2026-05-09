/*
 * starter-core - oil.c
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

#define ST_OIL_SERVICE_LOG_DOMAIN "starter-oil-example"

#define LOGD(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE(...) g_log (ST_OIL_SERVICE_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

enum
{
  IN_OIL_PRICE,
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

static char *
fetch_real_oil_price (void)
{
  const gchar *url =
    "https://query1.finance.yahoo.com/v8/finance/chart/CL=F?interval=1m&range=1d";
  char *price_str = NULL;
  char *buffer = NULL;
  gssize bytes;
  GFile *file = NULL;
  GFileInputStream *fis = NULL;

  file = g_file_new_for_uri (url);
  fis = g_file_read (file, NULL, NULL);

  if (!fis)
    {
      if (file)
	g_object_unref (file);

      return NULL;
    }

  buffer = g_malloc0 (4096);
  bytes =
    g_input_stream_read (G_INPUT_STREAM (fis), buffer, 4095, NULL, NULL);

  if (bytes > 0)
    {
      char *search_key = "\"regularMarketPrice\":";
      char *pos = NULL;
      buffer[bytes] = '\0';

      pos = strstr (buffer, search_key);

      if (pos)
	{
	  char *end = NULL;

	  pos += strlen (search_key);
	  end = strpbrk (pos, ",}");

	  if (end)
	    {
	      size_t len = end - pos;
	      price_str = g_strndup (pos, len);
	    }
	}
      else
	LOGE ("missing field");
    }

  g_input_stream_close (G_INPUT_STREAM (fis), NULL, NULL);
  g_free (buffer);
  g_object_unref (fis);
  g_object_unref (file);

  return price_str;
}

static gint
tick (gpointer user_data)
{
  char *price = NULL;
  double v;

  price = fetch_real_oil_price ();

  if (price)
    {
      if (sscanf (price, "%lf", &v) == 1)
	set_input_val (inputs->pdata[IN_OIL_PRICE], v);

      g_free (price);
    }

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
		   st_input_new ("oil:price", "Current oil price",
				 ST_UNIT_NULL, 0, 1000, 0.000001, 0,
				 ST_IO_FLAG_NO_HISTORY));

  outputs = g_ptr_array_new_full (0, g_object_unref);

  server = st_server_new ("ST:OilExample", &version, inputs, outputs);

  // Port 0 to make dynamic allocation
  st_server_start (server, 0, &error);

  mdns = st_mdns_new ();

  st_mdns_publish_service (mdns, st_server_get_name (server), "_dbus._tcp",
			   st_server_get_port (server));

  g_timeout_add_seconds (5, tick, NULL);

  g_main_loop_run (gloop);

  return EXIT_SUCCESS;
}
