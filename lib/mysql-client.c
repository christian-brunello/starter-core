/*
 * libstarter-core - mysql-client.c
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

#include <glib.h>
#include <glib/gprintf.h>

#include <mysql/mysql.h>

#include <starter/error.h>
#include <starter/input.h>
#include <starter/output.h>

#include <starter/mysql-client.h>

#include "internals.h"

#define DB_NAME "STARTER"

typedef struct
{
  MYSQL *handle;
  gchar *source;
} STMYSQLClientPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (STMYSQLClient, st_mysql_client, G_TYPE_OBJECT)
#define ST_MYSQL_CLIENT_GET_PRIVATE(obj) \
    ((STMYSQLClientPrivate *) st_mysql_client_get_instance_private (ST_MYSQL_CLIENT (obj)))
     static void st_mysql_client_finalize (GObject * gobject)
{
  STMYSQLClient *self = ST_MYSQL_CLIENT (gobject);
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);

  LOGD ("finalize STMYSQLClient %p", self);

  mysql_close (priv->handle);
  g_free (priv->source);

  G_OBJECT_CLASS (st_mysql_client_parent_class)->finalize (gobject);
}

static void
st_mysql_client_class_init (STMYSQLClientClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  LOGD ("Initialize STMYSQLClientClass %p", klass);

  object_class->finalize = st_mysql_client_finalize;
}

static void
st_mysql_client_init (STMYSQLClient * self)
{
  LOGD ("Initialize STMYSQLClient %p", self);
}

static gboolean
st_mysql_client_perform_query (STMYSQLClient * self, const char *q,
			       GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gboolean r = FALSE;

  if (mysql_query (priv->handle, q) != 0)
    {
      LOGE ("error execute SQL Query: %s", q);
      LOGE ("MYSql error: %s", mysql_error (priv->handle));

      g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE, "%s",
		   mysql_error (priv->handle));
    }
  else
    {
      LOGD ("query executed successfully: %s", q);
      r = TRUE;
    }

  return r;
}

STMYSQLClient *
st_mysql_client_new (const gchar * source, const gchar * host, guint16 port,
		     const char *user, const gchar * pass)
{
  STMYSQLClient *client = g_object_new (ST_TYPE_MYSQL_CLIENT, NULL);
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (client);

  priv->handle = mysql_init (NULL);
  priv->source = g_strdup (source);

  if (mysql_real_connect
      (priv->handle, host, user, pass, DB_NAME, port, NULL, 0) == NULL)
    {
      LOGE ("error connecting to %s:%d: %s", host, port,
	    mysql_error (priv->handle));
    }
  else
    {
      int reconnect = 1;

      mysql_options (priv->handle, MYSQL_OPT_RECONNECT, &reconnect);
    }




  return client;
}

gboolean
st_mysql_client_store_input_label (STMYSQLClient * self,
				   const STInput * input, GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gboolean r = FALSE;
  GString *s;

  s = g_string_new ("");

  g_string_printf (s,
		   "INSERT INTO %1$s.labels (name,description,type,unit,min,max,step) "
		   " VALUES ('%2$s','%7$s','STInput',%3$u,%4$lf,%5$lf,%6$lf)"
		   " ON DUPLICATE KEY UPDATE"
		   " description = '%7$s',"
		   " type = 'STInput',"
		   " unit = %3$u,"
		   " min = %4$lf,"
		   " max = %5$lf,"
		   " step = %6$lf;",
		   DB_NAME,
		   st_input_get_name (input),
		   st_input_get_unit (input),
		   st_input_get_min (input),
		   st_input_get_max (input),
		   st_input_get_step (input),
		   st_input_get_description (input));

  r = st_mysql_client_perform_query (self, s->str, error);

  g_string_free (s, TRUE);

  return r;
}

gboolean
st_mysql_client_store_output_label (STMYSQLClient * self,
				    const STOutput * output, GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gboolean r = FALSE;
  GString *s;

  s = g_string_new ("");

  g_string_printf (s,
		   "INSERT INTO %1$s.labels (name,description,type,unit,min,max,step) "
		   " VALUES ('%2$s','%7$s','STOutput',%3$u,%4$lf,%5$lf,%6$lf)"
		   " ON DUPLICATE KEY UPDATE"
		   " description = '%7$s',"
		   " type = 'STOutput',"
		   " unit = %3$u,"
		   " min = %4$lf,"
		   " max = %5$lf,"
		   " step = %6$lf;",
		   DB_NAME,
		   st_output_get_name (output),
		   st_output_get_unit (output),
		   st_output_get_min (output),
		   st_output_get_max (output),
		   st_output_get_step (output),
		   st_output_get_description (output));

  r = st_mysql_client_perform_query (self, s->str, error);

  g_string_free (s, TRUE);

  return r;
}

gboolean
st_mysql_client_store_history (STMYSQLClient * self, const gchar * name,
			       gdouble value, guint64 flags, GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gboolean r = FALSE;
  GString *s;

  s = g_string_new ("");

  g_string_printf (s,
		   "INSERT INTO %s.history (source,label_id,value,flags) VALUES('%s',(SELECT id FROM labels WHERE name='%s'),%lf,%"
		   PRIu64 ");", DB_NAME, priv->source, name, value, flags);

  r = st_mysql_client_perform_query (self, s->str, error);

  g_string_free (s, TRUE);

  return r;
}

gdouble
st_mysql_client_get_last_history_value (STMYSQLClient * self,
					const char *name, GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gdouble r = ST_INVALID_VALUE;
  GString *s;

  s = g_string_new ("");

  g_string_printf (s,
		   "SELECT value FROM %s.history WHERE label_id=(SELECT id FROM %s.labels WHERE name='%s') "
		   "ORDER BY timestamp DESC LIMIT 1;",
		   DB_NAME, DB_NAME, name);

  if (st_mysql_client_perform_query (self, s->str, error))
    {
      MYSQL_RES *result;

      if ((result = mysql_store_result (priv->handle)) != NULL)
	{
	  if (mysql_num_rows (result) == 1)
	    {
	      MYSQL_ROW row;

	      row = mysql_fetch_row (result);

	      if (row[0] != NULL)
		sscanf (row[0], "%lf", &r);
	    }

	  mysql_free_result (result);
	}
    }

  g_string_free (s, TRUE);

  return r;
}

gboolean
st_mysql_client_store_stats (STMYSQLClient * self, const STStats * stats,
			     GError ** error)
{
  STMYSQLClientPrivate *priv = ST_MYSQL_CLIENT_GET_PRIVATE (self);
  gboolean r = FALSE;
  const gchar *name = NULL;
  STStatsEntry vmsize;
  STStatsEntry vmrss;
  STStatsEntry vmswap;
  STStatsEntry io_read;
  STStatsEntry io_write;
  STStatsEntry priority;
  STStatsEntry threads;
  STStatsEntry icswitch;
  STStatsEntry cpu;
  GString *s;

  st_stats_get_full (stats,
		     &name,
		     &vmsize,
		     &vmrss,
		     &vmswap,
		     &io_read,
		     &io_write, &priority, &threads, &icswitch, &cpu);

  s = g_string_new ("");

  g_string_printf (s,
		   "INSERT INTO %1$s.stats (service_name,"
		   "  vmsize_min,vmsize_max,vmsize_last,"
		   "  vmrss_min,vmrss_max,vmrss_last,"
		   "  vmswap_min,vmswap_max,vmswap_last,"
		   "  io_read_min,io_read_max,io_read_last,"
		   "  io_write_min,io_write_max,io_write_last,"
		   "  cpu_min,cpu_max,cpu_last,"
		   "  icswitch_min,icswitch_max,icswitch_last,"
		   "  priority_min,priority_max,priority_last,"
		   "  threads_min,threads_max,threads_last) "
		   "VALUES('%2$s',"
		   "  %3$lf,%4$lf,%5$lf,"
		   "  %6$lf,%7$lf,%8$lf,"
		   "  %9$lf,%10$lf,%11$lf,"
		   "  %12$lf,%13$lf,%14$lf,"
		   "  %15$lf,%16$lf,%17$lf,"
		   "  %18$lf,%19$lf,%20$lf,"
		   "  %21$lf,%22$lf,%23$lf,"
		   "  %24$lf,%25$lf,%26$lf,"
		   "  %27$lf,%28$lf,%29$lf) "
		   "ON DUPLICATE KEY UPDATE "
		   "  updated_at=NOW(),"
		   "  vmsize_min=%3$lf,vmsize_max=%4$lf,vmsize_last=%5$lf,"
		   "  vmrss_min=%6$lf,vmrss_max=%7$lf,vmrss_last=%8$lf,"
		   "  vmswap_min=%9$lf,vmswap_max=%10$lf,vmswap_last=%11$lf,"
		   "  io_read_min=%12$lf,io_read_max=%13$lf,io_read_last=%14$lf,"
		   "  io_write_min=%15$lf,io_write_max=%16$lf,io_write_last=%17$lf,"
		   "  cpu_min=%18$lf,cpu_max=%19$lf,cpu_last=%20$lf,"
		   "  icswitch_min=%21$lf,icswitch_max=%22$lf,icswitch_last=%23$lf,"
		   "  priority_min=%24$lf,priority_max=%25$lf,priority_last=%26$lf,"
		   "  threads_min=%27$lf,threads_max=%28$lf,threads_last=%29$lf;",
		   DB_NAME,
		   name,
		   vmsize.min, vmsize.max, vmsize.last,
		   vmrss.min, vmrss.max, vmrss.last,
		   vmswap.min, vmswap.max, vmswap.last,
		   io_read.min, io_read.max, io_read.last,
		   io_write.min, io_write.max, io_write.last,
		   cpu.min, cpu.max, cpu.last,
		   icswitch.min, icswitch.max, icswitch.last,
		   priority.min, priority.max, priority.last,
		   threads.min, threads.max, threads.last);

  r = st_mysql_client_perform_query (self, s->str, error);

  g_string_free (s, TRUE);

  return r;
}
