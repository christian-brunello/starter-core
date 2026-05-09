/*
 * libstarter-core - stats.h
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

#ifndef STARTER_STATS_H_INCLUDED
#define STARTER_STATS_H_INCLUDED

#include <glib-object.h>

G_BEGIN_DECLS typedef struct
{
  double min;
  double max;
  double last;
} STStatsEntry;

#define ST_TYPE_STATS (st_stats_get_type ())

G_DECLARE_FINAL_TYPE (STStats, st_stats, ST, STATS, GObject)
     STStats *st_stats_new (const gchar * name);
     STStats *st_stats_dup (const STStats * self);


     const gchar *st_stats_get_name (const STStats * self);
     const STStatsEntry *st_stats_get_vmsize (const STStats * self);
     const STStatsEntry *st_stats_get_vmrss (const STStats * self);
     const STStatsEntry *st_stats_get_vmswap (const STStats * self);
     const STStatsEntry *st_stats_get_io_read (const STStats * self);
     const STStatsEntry *st_stats_get_io_write (const STStats * self);
     const STStatsEntry *st_stats_get_priority (const STStats * self);
     const STStatsEntry *st_stats_get_threads (const STStats * self);
     const STStatsEntry *st_stats_get_icswitch (const STStats * self);
     const STStatsEntry *st_stats_get_cpu (const STStats * self);
     void st_stats_get_full (const STStats * self,
			     const gchar ** name,
			     STStatsEntry * vmsize,
			     STStatsEntry * vmrss,
			     STStatsEntry * vmswap,
			     STStatsEntry * io_read,
			     STStatsEntry * io_write,
			     STStatsEntry * priority,
			     STStatsEntry * threads,
			     STStatsEntry * icswitch, STStatsEntry * cpu);


     void st_stats_set_name (STStats * self, const gchar * v);
     void st_stats_set_vmsize (STStats * self, const STStatsEntry * v);
     void st_stats_set_vmrss (STStats * self, const STStatsEntry * v);
     void st_stats_set_vmswap (STStats * self, const STStatsEntry * v);
     void st_stats_set_io_read (STStats * self, const STStatsEntry * v);
     void st_stats_set_io_write (STStats * self, const STStatsEntry * v);
     void st_stats_set_priority (STStats * self, const STStatsEntry * v);
     void st_stats_set_threads (STStats * self, const STStatsEntry * v);

     void st_stats_set_icswitch (STStats * self, const STStatsEntry * v);
     void st_stats_set_cpu (STStats * self, const STStatsEntry * v);
     void st_stats_set_full (STStats * self,
			     const gchar * name,
			     const STStatsEntry * vmsize,
			     const STStatsEntry * vmrss,
			     const STStatsEntry * vmswap,
			     const STStatsEntry * io_read,
			     const STStatsEntry * io_write,
			     const STStatsEntry * priority,
			     const STStatsEntry * threads,
			     const STStatsEntry * icswitch,
			     const STStatsEntry * cpu);

     G_END_DECLS void st_entry_init (STStatsEntry * entry);
     void st_entry_update (STStatsEntry * entry, double val);

#endif /* STARTER_STATS_H_INCLUDED */
