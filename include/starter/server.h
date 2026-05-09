/*
 * libstarter-core - server.h
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

#ifndef STARTER_SERVER_H_INCLUDED
#define STARTER_SERVER_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include <starter/version.h>

G_BEGIN_DECLS
#define ST_TYPE_SERVER (st_server_get_type ())
G_DECLARE_DERIVABLE_TYPE (STServer, st_server, ST, SERVER, GObject)
     struct _STServerClass
     {
       GObjectClass parent_class;

       gpointer padding[0x08];
     };

     STServer *st_server_new (const gchar * name, const STVersion * version,
			      GPtrArray * inputs, GPtrArray * outputs);

     const gchar *st_server_get_name (const STServer * self);
     gchar *st_server_dup_name (const STServer * self);
     gboolean st_server_set_name (STServer * self, const gchar * name,
				  GError ** error);

     const STVersion *st_server_get_version (const STServer * self);
     STVersion *st_server_dup_version (const STServer * self);
     gboolean st_server_set_version (STServer * self,
				     const STVersion * version,
				     GError ** error);

     const GPtrArray *st_server_get_inputs (const STServer * self);
     GPtrArray *st_server_dup_inputs (const STServer * self);
     gboolean st_server_set_inputs (STServer * self, GPtrArray * inputs);

     const GPtrArray *st_server_get_outputs (const STServer * self);
     GPtrArray *st_server_dup_outputs (const STServer * self);
     gboolean st_server_set_outputs (STServer * self, GPtrArray * outputs,
				     GError * error);

     gboolean st_server_start (STServer * self, guint16 port,
			       GError ** error);

     guint16 st_server_get_port (const STServer * self);

G_END_DECLS
#endif /* STARTER_SERVER_H_INCLUDED */
