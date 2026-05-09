/*
 * libstarter-core - stats.c
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

#include <starter/stats.h>

#include "internals.h"

#define VMSIZE_TH 1
#define VMRSS_TH 1
#define VMSWAP_TH 1
#define IO_READ_TH 1
#define IO_WRITE_TH 1
#define PRIORITY_TH 1
#define THREADS_TH 1
#define ICSWITCH_TH 1
#define CPU_TH 1

struct _STStats
{
  GObject parent_object;
  gchar *name;
  STStatsEntry vmsize;
  STStatsEntry vmrss;
  STStatsEntry vmswap;
  STStatsEntry io_read;
  STStatsEntry io_write;
  STStatsEntry priority;
  STStatsEntry threads;
  STStatsEntry icswitch;
  STStatsEntry cpu;
};

enum
{
  ST_STATS_SIGNAL_CHANGED,
  ST_STATS_SIGNAL_COUNT
};

static gint st_stats_signals[ST_STATS_SIGNAL_COUNT];

G_DEFINE_TYPE (STStats, st_stats, G_TYPE_OBJECT)
     static void st_stats_emit_changed_signal (STStats * self);

     static void dump_one_entry (const STStatsEntry * e, const char *id)
{
  LOGD ("%s: STStatsEntry *%p {min: %lf, max: %lf, last: %lf}", id, e, e->min,
	e->max, e->last);
}

static gboolean
entry_diff (const STStatsEntry * oe, const STStatsEntry * ne, double th)
{
  return fabs (oe->min - ne->min) >= th
    || fabs (oe->max - ne->max) >= th || fabs (oe->last - ne->last) >= th;
}

static void
st_stats_emit_changed_signal (STStats * self)
{
  g_signal_emit (self, st_stats_signals[ST_STATS_SIGNAL_CHANGED], 0);
}

static void
st_stats_finalize (GObject * object)
{
  STStats *self = ST_STATS (object);

  LOGD ("finalize STStats %p", object);

  g_free (self->name);

  G_OBJECT_CLASS (st_stats_parent_class)->finalize (object);
}

static void
st_stats_class_init (STStatsClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = st_stats_finalize;

  st_stats_signals[ST_STATS_SIGNAL_CHANGED] = g_signal_new ("changed",
							    G_TYPE_FROM_CLASS
							    (klass),
							    G_SIGNAL_RUN_LAST,
							    0, NULL, NULL,
							    NULL, G_TYPE_NONE,
							    0);
}

static void
st_stats_init (STStats * self)
{
  self->name = NULL;
  st_entry_init (&self->vmsize);
  st_entry_init (&self->vmrss);
  st_entry_init (&self->vmswap);
  st_entry_init (&self->io_read);
  st_entry_init (&self->io_write);
  st_entry_init (&self->priority);
  st_entry_init (&self->threads);
  st_entry_init (&self->icswitch);
  st_entry_init (&self->cpu);
}

STStats *
st_stats_new (const gchar * name)
{
  STStats *r;

  r = g_object_new (ST_TYPE_STATS, NULL);

  r->name = g_strdup (name);

  return r;
}

STStats *
st_stats_dup (const STStats * self)
{
  STStats *r;

  r = g_object_new (ST_TYPE_STATS, NULL);

  r->name = g_strdup (self->name);

  memcpy (&r->vmsize, &self->vmsize, sizeof (STStatsEntry));
  memcpy (&r->vmrss, &self->vmrss, sizeof (STStatsEntry));
  memcpy (&r->vmswap, &self->vmswap, sizeof (STStatsEntry));
  memcpy (&r->io_read, &self->io_read, sizeof (STStatsEntry));
  memcpy (&r->io_write, &self->io_write, sizeof (STStatsEntry));
  memcpy (&r->priority, &self->priority, sizeof (STStatsEntry));
  memcpy (&r->threads, &self->threads, sizeof (STStatsEntry));
  memcpy (&r->icswitch, &self->icswitch, sizeof (STStatsEntry));
  memcpy (&r->cpu, &self->cpu, sizeof (STStatsEntry));

  return r;
}

const gchar *
st_stats_get_name (const STStats * self)
{
  return self->name;
}

const gchar *
st_stats_dup_name (const STStats * self)
{
  return g_strdup (self->name);
}

const STStatsEntry *
st_stats_get_vmsize (const STStats * self)
{
  return &self->vmsize;
}

const STStatsEntry *
st_stats_get_vmrss (const STStats * self)
{
  return &self->vmrss;
}

const STStatsEntry *
st_stats_get_vmswap (const STStats * self)
{
  return &self->vmswap;
}

const STStatsEntry *
st_stats_get_io_read (const STStats * self)
{
  return &self->io_read;
}

const STStatsEntry *
st_stats_get_io_write (const STStats * self)
{
  return &self->io_write;
}

const STStatsEntry *
st_stats_get_priority (const STStats * self)
{
  return &self->priority;
}

const STStatsEntry *
st_stats_get_threads (const STStats * self)
{
  return &self->threads;
}

const STStatsEntry *
st_stats_get_icswitch (const STStats * self)
{
  return &self->icswitch;
}

const STStatsEntry *
st_stats_get_cpu (const STStats * self)
{
  return &self->cpu;
}

void
st_stats_get_full (const STStats * self,
		   const gchar ** name,
		   STStatsEntry * vmsize,
		   STStatsEntry * vmrss,
		   STStatsEntry * vmswap,
		   STStatsEntry * io_read,
		   STStatsEntry * io_write,
		   STStatsEntry * priority,
		   STStatsEntry * threads,
		   STStatsEntry * icswitch, STStatsEntry * cpu)
{
  if (name)
    *name = self->name;

  if (vmsize)
    memcpy (vmsize, &self->vmsize, sizeof (STStatsEntry));

  if (vmrss)
    memcpy (vmrss, &self->vmrss, sizeof (STStatsEntry));

  if (vmswap)
    memcpy (vmswap, &self->vmswap, sizeof (STStatsEntry));

  if (io_read)
    memcpy (io_read, &self->io_read, sizeof (STStatsEntry));

  if (io_write)
    memcpy (io_write, &self->io_write, sizeof (STStatsEntry));

  if (priority)
    memcpy (priority, &self->priority, sizeof (STStatsEntry));

  if (threads)
    memcpy (threads, &self->threads, sizeof (STStatsEntry));

  if (icswitch)
    memcpy (icswitch, &self->icswitch, sizeof (STStatsEntry));

  if (cpu)
    memcpy (cpu, &self->cpu, sizeof (STStatsEntry));
}

void
st_stats_set_name (STStats * self, const gchar * v)
{
  gboolean changed = strcmp (self->name, v) != 0;

  g_free (self->name);

  self->name = g_strdup (v);

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_vmsize (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->vmsize, v, VMSIZE_TH);

  memcpy (&self->vmsize, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_vmrss (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->vmrss, v, VMRSS_TH);

  memcpy (&self->vmrss, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_vmswap (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->vmswap, v, VMSWAP_TH);

  memcpy (&self->vmswap, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_io_read (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->io_read, v, IO_READ_TH);

  memcpy (&self->io_read, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_io_write (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->io_write, v, IO_WRITE_TH);

  memcpy (&self->io_write, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_priority (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->priority, v, PRIORITY_TH);

  memcpy (&self->priority, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_threads (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->threads, v, THREADS_TH);

  memcpy (&self->threads, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_icswitch (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->icswitch, v, ICSWITCH_TH);

  memcpy (&self->icswitch, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_cpu (STStats * self, const STStatsEntry * v)
{
  gboolean changed = entry_diff (&self->cpu, v, CPU_TH);

  memcpy (&self->cpu, v, sizeof (STStatsEntry));

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_stats_set_full (STStats * self,
		   const gchar * name,
		   const STStatsEntry * vmsize,
		   const STStatsEntry * vmrss,
		   const STStatsEntry * vmswap,
		   const STStatsEntry * io_read,
		   const STStatsEntry * io_write,
		   const STStatsEntry * priority,
		   const STStatsEntry * threads,
		   const STStatsEntry * icswitch, const STStatsEntry * cpu)
{
  guint changed = 0;

  if (name)
    {
      if (strcmp (self->name, name) != 0)
	changed++;

      g_free (self->name);
      self->name = g_strdup (name);
    }

  if (vmsize)
    {
      if (entry_diff (&self->vmsize, vmsize, VMSIZE_TH))
	changed++;

      memcpy (&self->vmsize, vmsize, sizeof (STStatsEntry));
    }

  if (vmrss)
    {
      if (entry_diff (&self->vmrss, vmrss, VMRSS_TH))
	changed++;

      memcpy (&self->vmrss, vmrss, sizeof (STStatsEntry));
    }

  if (vmswap)
    {
      if (entry_diff (&self->vmswap, vmswap, VMSWAP_TH))
	changed++;

      memcpy (&self->vmswap, vmswap, sizeof (STStatsEntry));
    }

  if (io_read)
    {
      if (entry_diff (&self->io_read, io_read, IO_READ_TH))
	changed++;

      memcpy (&self->io_read, io_read, sizeof (STStatsEntry));
    }

  if (io_write)
    {
      if (entry_diff (&self->io_write, io_write, IO_WRITE_TH))
	changed++;

      memcpy (&self->io_write, io_write, sizeof (STStatsEntry));
    }

  if (priority)
    {
      if (entry_diff (&self->priority, priority, PRIORITY_TH))
	changed++;

      memcpy (&self->priority, priority, sizeof (STStatsEntry));
    }

  if (threads)
    {
      if (entry_diff (&self->threads, threads, THREADS_TH))
	changed++;

      memcpy (&self->threads, threads, sizeof (STStatsEntry));
    }

  if (icswitch)
    {
      if (entry_diff (&self->icswitch, icswitch, ICSWITCH_TH))
	changed++;

      memcpy (&self->icswitch, icswitch, sizeof (STStatsEntry));
    }

  if (cpu)
    {
      if (entry_diff (&self->cpu, cpu, CPU_TH))
	changed++;

      memcpy (&self->cpu, cpu, sizeof (STStatsEntry));
    }

  if (changed)
    st_stats_emit_changed_signal (self);
}

void
st_entry_init (STStatsEntry * entry)
{
  entry->min = G_MAXDOUBLE;
  entry->max = G_MINDOUBLE;
  entry->last = 0;
}

void
st_entry_update (STStatsEntry * entry, double val)
{
  if (val < entry->min)
    entry->min = val;

  if (val > entry->max)
    entry->max = val;

  entry->last = val;
}
