/*
 * libstarter-core - mdns-service.c
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

#include <starter/mdns-service.h>

#include "internals.h"

struct _STMDNSService
{
  GObject parent_object;
};

typedef struct
{
  gchar *name;
  gchar *type;
  gchar *host;
  gchar *address;
  guint16 port;
  gint proto;
} STMDNSServicePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (STMDNSService, st_mdns_service, G_TYPE_OBJECT)
#define ST_MDNS_SERVICE_GET_PRIVATE(obj) \
    ((STMDNSServicePrivate *) st_mdns_service_get_instance_private (ST_MDNS_SERVICE (obj)))
     static void st_mdns_service_finalize (GObject * object)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (object);

  LOGD ("finalize STMDNSService %p", object);

  g_free (priv->name);
  g_free (priv->type);
  g_free (priv->host);
  g_free (priv->address);

  G_OBJECT_CLASS (st_mdns_service_parent_class)->finalize (object);
}

static void
st_mdns_service_class_init (STMDNSServiceClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = st_mdns_service_finalize;
}

static void
st_mdns_service_init (STMDNSService * self)
{

}

STMDNSService *
st_mdns_service_new (const gchar * name, const gchar * type,
		     const gchar * host, const gchar * address,
		     const guint16 port, const gint proto)
{
  STMDNSService *r;
  STMDNSServicePrivate *priv;

  r = g_object_new (ST_TYPE_MDNS_SERVICE, NULL);
  priv = ST_MDNS_SERVICE_GET_PRIVATE (r);

  priv->name = g_strdup (name);
  priv->type = g_strdup (type);
  priv->host = g_strdup (host);
  priv->address = g_strdup (address);
  priv->port = port;
  priv->proto = proto;

  return r;
}

gint
st_mdns_service_compare (const STMDNSService * self,
			 const STMDNSService * other)
{
  const STMDNSServicePrivate *self_priv;
  const STMDNSServicePrivate *other_priv;
  gint r = -1;

  self_priv = ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);
  other_priv = ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) other);

  if ((r = strcmp (self_priv->name, other_priv->name)) == 0)
    {
      if ((r = strcmp (self_priv->type, other_priv->type)) == 0)
	{
	  if ((r = strcmp (self_priv->host, other_priv->host)) == 0)
	    {
	      if ((r = strcmp (self_priv->address, other_priv->address)) == 0)
		{
		  if (self_priv->port == other_priv->port)
		    {
		      r =
			self_priv->proto <
			other_priv->proto ? -1 : self_priv->proto >
			other_priv->proto ? 1 : 0;
		    }
		  else
		    r = self_priv->port < other_priv->port ? -1 : 1;
		}
	    }
	}
    }

  return r;
}


const gchar *
st_mdns_service_get_name (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->name;
}

gchar *
st_mdns_service_dup_name (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return g_strdup (priv->name);
}

void
st_mdns_service_set_name (STMDNSService * self, const gchar * name)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  g_free (priv->name);
  priv->name = g_strdup (name);
}

const gchar *
st_mdns_service_get_mdns_type (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->type;
}

gchar *
st_mdns_service_dup_mdns_type (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return g_strdup (priv->type);
}

void
st_mdns_service_set_mdns_type (STMDNSService * self, const gchar * type)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  g_free (priv->type);
  priv->type = g_strdup (type);
}

const gchar *
st_mdns_service_get_host (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->host;
}

gchar *
st_mdns_service_dup_host (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return g_strdup (priv->host);
}

void
st_mdns_service_set_host (STMDNSService * self, const gchar * host)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  g_free (priv->host);
  priv->host = g_strdup (host);
}

const gchar *
st_mdns_service_get_address (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->address;
}

gchar *
st_mdns_service_dup_address (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return g_strdup (priv->address);
}

void
st_mdns_service_set_address (STMDNSService * self, const gchar * address)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  g_free (priv->address);
  priv->address = g_strdup (address);
}

guint16
st_mdns_service_get_port (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->port;
}

void
st_mdns_service_set_port (STMDNSService * self, guint16 port)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  priv->port = port;
}

gint
st_mdns_service_get_proto (const STMDNSService * self)
{
  STMDNSServicePrivate *priv =
    ST_MDNS_SERVICE_GET_PRIVATE ((STMDNSService *) self);

  return priv->proto;
}

void
st_mdns_service_set_proto (STMDNSService * self, guint16 proto)
{
  STMDNSServicePrivate *priv = ST_MDNS_SERVICE_GET_PRIVATE (self);

  priv->proto = proto;
}
