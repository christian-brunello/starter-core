/*
 * libstarter-core - process-stats-collector.h
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

#ifndef STARTER_PROCESS_STATS_COLLECTOR_H_INCLUDED
#define STARTER_PROCESS_STATS_COLLECTOR_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include <starter/stats-collector.h>

G_BEGIN_DECLS
#define ST_TYPE_PROCESS_STATS_COLLECTOR (st_process_stats_collector_get_type ())
G_DECLARE_FINAL_TYPE (STProcessStatsCollector, st_process_stats_collector, ST,
		      PROCESS_STATS_COLLECTOR, STStatsCollector)
     STProcessStatsCollector *st_process_stats_collector_new (STStats * stats,
							      guint
							      interval_ms);

G_END_DECLS
#endif /* STARTER_PROCESS_STATS_COLLECTOR_H_INCLUDED */
