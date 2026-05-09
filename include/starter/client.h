/*
 * libstarter-core - client.h
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

#ifndef STARTER_CLIENT_H_INCLUDED
#define STARTER_CLIENT_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include <starter/version.h>
#include <starter/stats.h>

G_BEGIN_DECLS
#define ST_TYPE_CLIENT (st_client_get_type ())
G_DECLARE_DERIVABLE_TYPE (STClient, st_client, ST, CLIENT, GObject)
     struct _STClientClass
     {
       GObjectClass parent_class;

       gpointer padding[0x08];
     };

     STClient *st_client_new (void);

     gboolean st_client_start (STClient * self, const gchar * address,
			       guint16 port, GError ** error);

     const GPtrArray *st_client_get_inputs (STClient * self);
     const GPtrArray *st_client_get_outputs (STClient * self);
     const STStats *st_client_get_stats (STClient * self);
     gchar *st_client_get_service_name (STClient * self);
     gboolean st_client_set_output (STClient * self, const gchar * name,
				    double value, guint64 flags);

G_END_DECLS
#endif /* STARTER_CLIENT_H_INCLUDED */
