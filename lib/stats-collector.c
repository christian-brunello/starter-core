/*
 * libstarter-core - stats-collector.c
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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#include <starter/stats-collector.h>

#include "internals.h"

enum
{
  PROP_0,
  PROP_STATS,
  PROP_INTERVAL,
  N_PROPERTIES
};

typedef struct
{
  STStats *stats;
  guint interval_ms;
  guint source_id;
} STStatsCollectorPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (STStatsCollector, st_stats_collector,
			    G_TYPE_OBJECT)
#define ST_STATS_COLLECTOR_GET_PRIVATE(obj) \
    ((STStatsCollectorPrivate *) st_stats_collector_get_instance_private (ST_STATS_COLLECTOR (obj)))
     static GParamSpec *st_stats_collector_properties[N_PROPERTIES] =
       { NULL, };

static void
dump_one_entry (const STStatsEntry * e, const char *id)
{
  LOGD ("%s: STStatsEntry *%p {min: %lf, max: %lf, last: %lf}", id, e, e->min,
	e->max, e->last);
}

static gboolean
periodic_callback (gpointer user_data)
{
  STStatsCollector *self = user_data;
  STStatsCollectorPrivate *priv = ST_STATS_COLLECTOR_GET_PRIVATE (self);
  STStatsCollectorClass *klass = ST_STATS_COLLECTOR_GET_CLASS (self);
  GError *error = NULL;

  if (klass->update && priv->stats)
    {
      if (!(*klass->update) (self, priv->stats, &error))
	{
	  LOGE ("error from instance update: %s",
		error ? error->message : "unknown error");
	  g_error_free (error);
	}
    }
  else
    LOGE ("update method 'update' is NULL");

  return G_SOURCE_CONTINUE;
}

static void
st_stats_collector_set_property (GObject * object,
				 guint prop_id,
				 const GValue * value, GParamSpec * pspec)
{
  STStatsCollector *self = ST_STATS_COLLECTOR (object);
  STStatsCollectorPrivate *priv = ST_STATS_COLLECTOR_GET_PRIVATE (self);

  LOGD ("set property %d", prop_id);

  switch (prop_id)
    {
    case PROP_STATS:
      if (priv->stats)
	g_object_unref (priv->stats);

      priv->stats = g_object_ref (g_value_get_object (value));
      break;
    case PROP_INTERVAL:
      priv->interval_ms = g_value_get_uint (value);

      if (priv->source_id)
	g_source_remove (priv->source_id);

      priv->source_id =
	g_timeout_add (priv->interval_ms, periodic_callback, self);
      break;
    default:
      LOGE ("unmanaged property: %u", prop_id);
      break;
    }
}

static void
st_stats_collector_get_property (GObject * object,
				 guint prop_id,
				 GValue * value, GParamSpec * pspec)
{
  STStatsCollector *self = ST_STATS_COLLECTOR (object);
  STStatsCollectorPrivate *priv = ST_STATS_COLLECTOR_GET_PRIVATE (self);

  switch (prop_id)
    {
    case PROP_STATS:
      g_value_set_object (value, priv->stats);
      break;
    case PROP_INTERVAL:
      g_value_set_uint (value, priv->interval_ms);
      break;
    default:
      LOGE ("unmanaged property: %u", prop_id);
      break;
    }
}

static void
st_stats_collector_finalize (GObject * object)
{
  STStatsCollectorPrivate *priv = ST_STATS_COLLECTOR_GET_PRIVATE (object);

  LOGD ("finalize STStats %p", object);

  if (priv->stats)
    g_object_unref (priv->stats);

  if (priv->source_id > 0)
    g_source_remove (priv->source_id);

  G_OBJECT_CLASS (st_stats_collector_parent_class)->finalize (object);
}

static void
st_stats_collector_class_init (STStatsCollectorClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = st_stats_collector_set_property;
  gobject_class->get_property = st_stats_collector_get_property;
  gobject_class->finalize = st_stats_collector_finalize;

  st_stats_collector_properties[PROP_STATS] =
    g_param_spec_object ("stats",
			 "Stats",
			 "Updated Stats",
			 ST_TYPE_STATS,
			 G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  st_stats_collector_properties[PROP_INTERVAL] =
    g_param_spec_uint ("interval",
		       "Interval",
		       "Update interval in milliseconds",
		       500,
		       5000, 1000, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (gobject_class,
				     N_PROPERTIES,
				     st_stats_collector_properties);
}

static void
st_stats_collector_init (STStatsCollector * self)
{
  STStatsCollectorPrivate *priv =
    st_stats_collector_get_instance_private (self);

  memset (priv, 0x00, sizeof (STStatsCollectorPrivate));
}
