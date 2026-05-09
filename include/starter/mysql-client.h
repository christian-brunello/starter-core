/*
 * libstarter-core - mysql-client.h
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

#ifndef STARTER_MYSQL_CLIENT_H_INCLUDED
#define STARTER_MYSQL_CLIENT_H_INCLUDED

#include <glib-object.h>

#include <starter/stats.h>
#include <starter/input.h>
#include <starter/output.h>

#define ST_TYPE_MYSQL_CLIENT (st_mysql_client_get_type ())

G_DECLARE_DERIVABLE_TYPE (STMYSQLClient, st_mysql_client, ST, MYSQL_CLIENT,
			  GObject)
     struct _STMYSQLClientClass
     {
       GObjectClass parent_class;

       gpointer padding[0x08];
     };

     STMYSQLClient *st_mysql_client_new (const gchar * source,
					 const gchar * host, guint16 port,
					 const char *user,
					 const gchar * pass);
     gboolean st_mysql_client_store_input_label (STMYSQLClient * self,
						 const STInput * input,
						 GError ** error);
     gboolean st_mysql_client_store_output_label (STMYSQLClient * self,
						  const STOutput * output,
						  GError ** error);
     gboolean st_mysql_client_store_history (STMYSQLClient * self,
					     const gchar * name,
					     gdouble value, guint64 flags,
					     GError ** error);
     gdouble st_mysql_client_get_last_history_value (STMYSQLClient * self,
						     const char *name,
						     GError ** error);
     gboolean st_mysql_client_store_stats (STMYSQLClient * self,
					   const STStats * stats,
					   GError ** error);

#endif /* STARTER_MYSQL_CLIENT_H_INCLUDED */
