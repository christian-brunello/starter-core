/*
 * libstarter-core - mdns-service.h
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

#ifndef STARTER_MDNS_SERVICE_H_INCLUDED
#define STARTER_MDNS_SERVICE_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define ST_TYPE_MDNS_SERVICE (st_mdns_service_get_type ())
G_DECLARE_FINAL_TYPE (STMDNSService, st_mdns_service, ST, MDNS_SERVICE,
		      GObject)
     STMDNSService *st_mdns_service_new (const gchar * name,
					 const gchar * type,
					 const gchar * host,
					 const gchar * address,
					 const guint16 port,
					 const gint proto);

     const gchar *st_mdns_service_get_name (const STMDNSService * self);
     gchar *st_mdns_service_dup_name (const STMDNSService * self);
     void st_mdns_service_set_name (STMDNSService * self, const gchar * name);
     const gchar *st_mdns_service_get_mdns_type (const STMDNSService * self);
     gchar *st_mdns_service_dup_mdns_type (const STMDNSService * self);
     void st_mdns_service_set_mdns_type (STMDNSService * self,
					 const gchar * type);
     const gchar *st_mdns_service_get_host (const STMDNSService * self);
     gchar *st_mdns_service_dup_host (const STMDNSService * self);
     void st_mdns_service_set_host (STMDNSService * self, const gchar * host);
     const gchar *st_mdns_service_get_address (const STMDNSService * self);
     gchar *st_mdns_service_dup_address (const STMDNSService * self);
     void st_mdns_service_set_address (STMDNSService * self,
				       const gchar * address);
     guint16 st_mdns_service_get_port (const STMDNSService * self);
     void st_mdns_service_set_port (STMDNSService * self, guint16 port);
     gint st_mdns_service_get_proto (const STMDNSService * self);
     void st_mdns_service_set_proto (STMDNSService * self, guint16 proto);
;
     gint st_mdns_service_compare (const STMDNSService * self,
				   const STMDNSService * other);

G_END_DECLS
#endif /* ST_MDNS_SERVICE_H_INCLUDED */
