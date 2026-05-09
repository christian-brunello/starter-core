/*
 * libstarter-core - mdns.c
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

#include <stdio.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/address.h>

#include <avahi-glib/glib-malloc.h>
#include <avahi-glib/glib-watch.h>

#include <glib.h>

#include <starter/mdns.h>

#include "internals.h"

struct _STMDNS
{
  GObject parent_instance;
  AvahiGLibPoll *poll;
  AvahiClient *client;
  AvahiEntryGroup *group;
  AvahiServiceTypeBrowser *service_type_browser;
  GHashTable *service_browsers;
  GList *services;
};

enum
{
  ST_MDNS_SIGNAL_SERVICE_ADDED,
  ST_MDNS_SIGNAL_SERVICE_REMOVED,
  ST_MDNS_SIGNAL_COUNT
};

static gint st_mdns_signals[ST_MDNS_SIGNAL_COUNT];

static void
avahi_service_browser_destroy (gpointer p)
{
  avahi_service_browser_free (p);
}

static const STMDNSService *
lookup_service (STMDNS * self, const gchar * name, const gchar * type,
		const gchar * host, const gchar * address, guint16 port,
		int proto)
{
  GList *node;
  STMDNSService *b;
  STMDNSService *r = NULL;

  b = st_mdns_service_new (name, type, host, address, port, proto);

  node =
    g_list_find_custom (self->services, b,
			(GCompareFunc) st_mdns_service_compare);

  if (node)
    r = node->data;

  g_object_unref (b);

  return r;
}

static void
remove_services_by_name_and_type (STMDNS * self, const gchar * name,
				  const gchar * type)
{
  gboolean done = FALSE;

  while (!done)
    {
      GList *new_services = self->services;
      gboolean removed = FALSE;

      while (new_services)
	{
	  STMDNSService *service = new_services->data;
	  gchar *xname;
	  gchar *xtype;

	  xname = st_mdns_service_dup_name (service);
	  xtype = st_mdns_service_dup_mdns_type (service);

	  if (strcmp (name, xname) == 0 && strcmp (type, xtype) == 0)
	    {
	      self->services = g_list_remove (self->services, service);

	      new_services = NULL;
	      removed = TRUE;
	    }
	  else
	    new_services = new_services->next;

	  g_free (xname);
	  g_free (xtype);
	}

      if (!removed)
	done = TRUE;
    }
}

static void
entry_group_callback (AvahiEntryGroup * g, AvahiEntryGroupState state,
		      AVAHI_GCC_UNUSED void *userdata)
{
  STMDNS *self = userdata;

  switch (state)
    {
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
      LOGD ("Group successfully established.");
      break;
    case AVAHI_ENTRY_GROUP_COLLISION:
      {
	LOGE ("Service name collision");
	break;
      }
    case AVAHI_ENTRY_GROUP_FAILURE:
      LOGE ("Entry group failure: %s\n",
	    avahi_strerror (avahi_client_errno
			    (avahi_entry_group_get_client (g))));
      break;
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
    case AVAHI_ENTRY_GROUP_REGISTERING:
      ;
    }
}

static void
resolve_cb (AvahiServiceResolver * r, AvahiIfIndex interface,
	    AvahiProtocol protocol, AvahiResolverEvent event,
	    const char *name, const char *type, const char *domain,
	    const char *host_name, const AvahiAddress * address,
	    uint16_t port, AvahiStringList * txt,
	    AvahiLookupResultFlags flags, gpointer userdata)
{

  STMDNS *self = userdata;

  avahi_service_resolver_free (r);

  if (event == AVAHI_RESOLVER_FOUND)
    {
      char addr_str[AVAHI_ADDRESS_STR_MAX];

      LOGD ("new service resolved: %s", name);

      avahi_address_snprint (addr_str, sizeof (addr_str), address);

      if (self->services == NULL
	  || lookup_service (self, name, type, host_name, addr_str, port,
			     avahi_proto_to_af (protocol)) == NULL)
	{
	  STMDNSService *p;

	  LOGD ("service not found in list, add");

	  p =
	    st_mdns_service_new (name, type, host_name, addr_str, port,
				 avahi_proto_to_af (protocol));

	  self->services = g_list_append (self->services, p);

	  g_signal_emit (self, st_mdns_signals[ST_MDNS_SIGNAL_SERVICE_ADDED],
			 0, p);

	  LOGI
	    ("new service: name: %s, type: %s, host: %s, address: %s, port: %hu, proto: %d",
	     name, type, host_name, addr_str, port, protocol);
	}
      else
	LOGD ("service already in list");

    }
  else if (event == AVAHI_RESOLVER_FAILURE)
    {
      LOGE ("Resolver failed for %s: %s", name,
	    avahi_strerror (avahi_client_errno (self->client)));
    }
}

static void
service_browser_cb (AvahiServiceBrowser * b,
		    AvahiIfIndex interface,
		    AvahiProtocol protocol,
		    AvahiBrowserEvent event,
		    const char *name,
		    const char *type,
		    const char *domain,
		    AvahiLookupResultFlags flags, gpointer userdata)
{

  STMDNS *self = userdata;

  if (event == AVAHI_BROWSER_NEW)
    {
      LOGD ("Found new service instance: %s (%s) on domain %s", name, type,
	    domain);

      if (avahi_service_resolver_new
	  (self->client, interface, protocol, name, type, domain,
	   AVAHI_PROTO_UNSPEC, 0, resolve_cb, self) != NULL)
	{
	  LOGD ("service resolver created successfully");
	}
      else
	{
	  LOGE ("Failed to create resolver for %s: %s", name,
		avahi_strerror (avahi_client_errno (self->client)));
	}
    }
  else if (event == AVAHI_BROWSER_REMOVE)
    {
      LOGD ("Service instance removed: %s", name);

      remove_services_by_name_and_type (self, name, type);

      g_signal_emit (self, st_mdns_signals[ST_MDNS_SIGNAL_SERVICE_REMOVED], 0,
		     name);
    }
  else if (event == AVAHI_BROWSER_ALL_FOR_NOW)
    {
      LOGD ("--- Finished listing current services for %s ---", type);
    }
  else if (event == AVAHI_BROWSER_FAILURE)
    {
      LOGE ("Browser failure: %s",
	    avahi_strerror (avahi_client_errno
			    (avahi_service_browser_get_client (b))));
    }
}

static void
service_type_browser_cb (AvahiServiceTypeBrowser * b,
			 AvahiIfIndex interface,
			 AvahiProtocol protocol,
			 AvahiBrowserEvent event,
			 const char *type,
			 const char *domain,
			 AvahiLookupResultFlags flags, void *userdata)
{

  STMDNS *self = userdata;

  switch (event)
    {

    case AVAHI_BROWSER_NEW:
      LOGD ("found new service type: %s on domain %s", type, domain);

      if (g_hash_table_lookup (self->service_browsers, type) == NULL)
	{
	  AvahiServiceBrowser *browser;

	  LOGD ("service type not found, create browser");

	  if ((browser =
	       avahi_service_browser_new (self->client, AVAHI_IF_UNSPEC,
					  AVAHI_PROTO_UNSPEC, type, domain, 0,
					  service_browser_cb, self)) != NULL)
	    {
	      g_hash_table_insert (self->service_browsers, g_strdup (type),
				   browser);

	      LOGD ("service browser created successfully");
	    }
	  else
	    {
	      LOGE ("error create service browser");
	    }
	}
      else
	LOGD ("service type already found, skip");
      break;
    case AVAHI_BROWSER_REMOVE:
      LOGD ("service type: %s removed from domain %s", type, domain);
      break;
    case AVAHI_BROWSER_FAILURE:
      LOGW ("service type browser failed: %s",
	    avahi_strerror (avahi_client_errno (self->client)));
      break;
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
      break;
    case AVAHI_BROWSER_ALL_FOR_NOW:
      break;
    }
}

static void
client_state_cb (AvahiClient * c, AvahiClientState state, gpointer userdata)
{
  STMDNS *self = userdata;

  switch (state)
    {
    case AVAHI_CLIENT_S_RUNNING:
      {
	LOGD ("avahi client running");

	if ((self->service_type_browser =
	     avahi_service_type_browser_new (c, AVAHI_IF_UNSPEC,
					     AVAHI_PROTO_UNSPEC, "", 0,
					     service_type_browser_cb,
					     self)) != NULL)
	  {
	    LOGD ("service type browser created successfully");
	  }
	else
	  {
	    LOGE ("error create avahi service browser");
	  }

	if ((self->group =
	     avahi_entry_group_new (c, entry_group_callback, self)) != NULL)
	  {
	    LOGD ("group created successfully");
	  }
	else
	  {
	    LOGE ("error create avahi group");
	  }
      }
      break;
    case AVAHI_CLIENT_FAILURE:
      LOGE ("avahi client failure: %s",
	    avahi_strerror (avahi_client_errno (c)));
      break;
    default:
      LOGW ("unmanaged state %d", state);
      break;
    }
}


G_DEFINE_TYPE (STMDNS, st_mdns, G_TYPE_OBJECT)
     static void st_mdns_dispose (GObject * gobject)
{
  STMDNS *self = ST_MDNS (gobject);

  G_OBJECT_CLASS (st_mdns_parent_class)->dispose (gobject);
}

static void
st_mdns_finalize (GObject * gobject)
{
  STMDNS *self = ST_MDNS (gobject);

  LOGD ("finalize STMDNS %p", gobject);

  g_hash_table_unref (self->service_browsers);

  if (self->service_type_browser)
    avahi_service_type_browser_free (self->service_type_browser);

  if (self->group)
    avahi_entry_group_free (self->group);

  g_list_free_full (self->services, g_object_unref);

  avahi_glib_poll_free (self->poll);

  G_OBJECT_CLASS (st_mdns_parent_class)->finalize (gobject);
}

static void
st_mdns_class_init (STMDNSClass * klass)
{
  GObjectClass *self_class = G_OBJECT_CLASS (klass);

  st_mdns_signals[ST_MDNS_SIGNAL_SERVICE_ADDED] =
    g_signal_new ("service-added", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1,
		  ST_TYPE_MDNS_SERVICE);

  st_mdns_signals[ST_MDNS_SIGNAL_SERVICE_REMOVED] =
    g_signal_new ("service-removed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1,
		  G_TYPE_STRING);

  self_class->dispose = st_mdns_dispose;
  self_class->finalize = st_mdns_finalize;
}

static void
st_mdns_init (STMDNS * self)
{
  if ((self->poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT)) != NULL)
    {
      int error;

      self->service_browsers =
	g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
			       g_object_unref);
      self->services = NULL;

      if ((self->client =
	   avahi_client_new (avahi_glib_poll_get (self->poll), 0,
			     client_state_cb, self, &error)) != NULL)
	{
	  LOGD ("avahi client created successfully");
	}
      else
	LOGE ("error creating avahi client: %s", avahi_strerror (error));
    }
  else
    LOGE ("error allocate glib poll");
}



STMDNS *
st_mdns_new (void)
{
  STMDNS *r;

  r = g_object_new (ST_TYPE_MDNS, NULL);

  return r;
}

const GList *
st_mdns_get_all_services (STMDNS * self)
{
  return self->services;
}

gboolean
st_mdns_publish_service (STMDNS * self, const gchar * name,
			 const gchar * type, guint16 port)
{
  gboolean r = TRUE;

  if (avahi_entry_group_add_service
      (self->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, type, NULL,
       NULL, port, NULL) < 0)
    r = FALSE;

  if (avahi_entry_group_commit (self->group) < 0)
    r = FALSE;

  return r;
}
