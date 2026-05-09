/*
 * libstarter-core - process-stats-collector.c
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

#include <starter/process-stats-collector.h>

#include "internals.h"

#define STAT_FILE_PATH "/proc/self/stat"

struct _STProcessStatsCollector
{
  STStatsCollector parent_instance;
};

typedef struct
{
  gulong utime_prev;
  gulong stime_prev;

  gulong nivcsw_prev;

  gint64 monotonic_time_prev;

  gulong vmsize_current_kb;
  gulong vmrss_current_kb;
  gulong vmswap_current_kb;

  gulong rchar_prev;
  gulong wchar_prev;

  long priority_current;

  long num_threads_current;
  long clk_tck;
} STProcessStatsCollectorPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (STProcessStatsCollector,
			    st_process_stats_collector,
			    ST_TYPE_STATS_COLLECTOR)
#define ST_PROCESS_STATS_COLLECTOR_GET_PRIVATE(obj) \
    ((STProcessStatsCollectorPrivate *) st_process_stats_collector_get_instance_private (ST_PROCESS_STATS_COLLECTOR (obj)))
#define SAMPLING_INTERVAL_MS 5000
#define SAMPLING_INTERVAL_S  5.0
     static void consume_cpu_time (int duration_ms);
     static gboolean read_proc_stat_process (STProcessStatsCollectorPrivate *
					     state, gulong * utime_out,
					     gulong * stime_out,
					     long *priority_out,
					     long *num_threads_out,
					     gulong * nivcsw_out);
     static gboolean read_proc_status_memory (STProcessStatsCollectorPrivate *
					      state);
     static gboolean read_proc_io (STProcessStatsCollectorPrivate * state,
				   gulong * rchar_out, gulong * wchar_out);
     static STProcessStatsCollectorPrivate *init_monitoring ();
     static double calculate_cpu_usage (STProcessStatsCollectorPrivate *
					state, gulong utime_curr,
					gulong stime_curr,
					gint64 monotonic_time_curr);
     static double
       calculate_context_switch_rate (STProcessStatsCollectorPrivate * state,
				      gulong nivcsw_curr);

     static void consume_cpu_time (int duration_ms)
{
  long end_time_ms = g_get_monotonic_time () / 1000 + duration_ms;
  volatile double result = 1.0;
  while ((g_get_monotonic_time () / 1000) < end_time_ms)
    {
      result = sin (result + 0.0000001);
    }
}

static gboolean
read_proc_stat_process (STProcessStatsCollectorPrivate * state,
			gulong * utime_out, gulong * stime_out,
			long *priority_out, long *num_threads_out,
			gulong * nivcsw_out)
{
  FILE *stat_file = fopen ("/proc/self/stat", "r");
  if (!stat_file)
    return FALSE;
  char buffer[4096];
  if (fgets (buffer, sizeof (buffer), stat_file) == NULL)
    {
      fclose (stat_file);
      return FALSE;
    }
  fclose (stat_file);

  char *p = buffer;
  char *p_start_numeric = strrchr (buffer, ')');
  if (!p_start_numeric)
    return FALSE;
  p = p_start_numeric + 1;

#define SKIP_FIELDS(ptr, count) \
        for (int i = 0; i < (count); i++) { \
            while (*(ptr) == ' ') (ptr)++; \
            if (*(ptr) == '\0') return FALSE; \
            while (*(ptr) != ' ' && *(ptr) != '\0') (ptr)++; \
        }

  SKIP_FIELDS (p, 11);

  int fields_read = sscanf (p, " %lu %lu", utime_out, stime_out);
  if (fields_read != 2)
    return FALSE;

  SKIP_FIELDS (p, 2);

  fields_read = sscanf (p, " %*d %*d %ld", priority_out);
  if (fields_read != 1)
    return FALSE;

  SKIP_FIELDS (p, 3);

  fields_read = sscanf (p, " %*d %ld", num_threads_out);
  if (fields_read != 1)
    return FALSE;

  SKIP_FIELDS (p, 2);

  SKIP_FIELDS (p, 2);

  SKIP_FIELDS (p, 19);

  fields_read = sscanf (p, " %lu", nivcsw_out);

  return fields_read == 1;
}

static gboolean
read_proc_status_memory (STProcessStatsCollectorPrivate * state)
{
  FILE *status_file = fopen ("/proc/self/status", "r");
  if (!status_file)
    return FALSE;
  char line[256];

  state->vmsize_current_kb = 0;
  state->vmrss_current_kb = 0;
  state->vmswap_current_kb = 0;

  while (fgets (line, sizeof (line), status_file) != NULL)
    {
      if (g_str_has_prefix (line, "VmSize:"))
	{
	  sscanf (line, "VmSize: %lu kB", &state->vmsize_current_kb);
	}
      else if (g_str_has_prefix (line, "VmRSS:"))
	{
	  sscanf (line, "VmRSS: %lu kB", &state->vmrss_current_kb);
	}
      else if (g_str_has_prefix (line, "VmSwap:"))
	{
	  sscanf (line, "VmSwap: %lu kB", &state->vmswap_current_kb);
	}
    }
  fclose (status_file);
  return state->vmrss_current_kb > 0;
}

static gboolean
read_proc_io (STProcessStatsCollectorPrivate * state, gulong * rchar_out,
	      gulong * wchar_out)
{
  FILE *io_file = fopen ("/proc/self/io", "r");
  if (!io_file)
    return FALSE;
  char line[256];
  gboolean rchar_found = FALSE;
  gboolean wchar_found = FALSE;

  while (fgets (line, sizeof (line), io_file) != NULL)
    {
      if (g_str_has_prefix (line, "rchar:"))
	{
	  sscanf (line, "rchar: %lu", rchar_out);
	  rchar_found = TRUE;
	}
      else if (g_str_has_prefix (line, "wchar:"))
	{
	  sscanf (line, "wchar: %lu", wchar_out);
	  wchar_found = TRUE;
	}
    }
  fclose (io_file);
  return rchar_found && wchar_found;
}

static STProcessStatsCollectorPrivate *
init_monitoring (STProcessStatsCollectorPrivate * state)
{
  state->clk_tck = sysconf (_SC_CLK_TCK);
  if (state->clk_tck <= 0)
    state->clk_tck = 100;

  gulong nivcsw_dummy = 0;

  if (!read_proc_stat_process (state,
			       &state->utime_prev, &state->stime_prev,
			       &state->priority_current,
			       &state->num_threads_current, &nivcsw_dummy))
    {
      LOGW ("error reading /proc/self/stat");
    }
  state->nivcsw_prev = nivcsw_dummy;
  state->monotonic_time_prev = g_get_monotonic_time ();

  read_proc_io (state, &state->rchar_prev, &state->wchar_prev);

  return state;
}

static double
calculate_cpu_usage (STProcessStatsCollectorPrivate * state,
		     gulong utime_curr, gulong stime_curr,
		     gint64 monotonic_time_curr)
{
  gulong proc_time_curr = utime_curr + stime_curr;
  gulong proc_time_prev = state->utime_prev + state->stime_prev;

  if (proc_time_curr < proc_time_prev)
    return 0.0;

  gulong delta_proc_time = proc_time_curr - proc_time_prev;
  gint64 delta_time_us = monotonic_time_curr - state->monotonic_time_prev;

  double cpu_percent = 0.0;

  if (delta_time_us > 0)
    {
      double delta_time_seconds = (double) delta_time_us / 1000000.0;
      double elapsed_time_jiffies = delta_time_seconds * state->clk_tck;

      if (elapsed_time_jiffies > 0.0)
	{
	  cpu_percent =
	    ((double) delta_proc_time / elapsed_time_jiffies) * 100.0;
	}
    }

  state->utime_prev = utime_curr;
  state->stime_prev = stime_curr;

  return cpu_percent;
}

static double
calculate_context_switch_rate (STProcessStatsCollectorPrivate * state,
			       gulong nivcsw_curr)
{
  if (nivcsw_curr < state->nivcsw_prev)
    return 0.0;

  gulong delta_nivcsw = nivcsw_curr - state->nivcsw_prev;
  double rate = (double) delta_nivcsw / SAMPLING_INTERVAL_S;

  state->nivcsw_prev = nivcsw_curr;

  return rate;
}

static gboolean
update_callback (STStatsCollector * obj, STStats * stats, GError ** error)
{
  STProcessStatsCollector *self;
  STProcessStatsCollectorPrivate *priv;
  STStatsEntry vmsize;
  STStatsEntry vmrss;
  STStatsEntry vmswap;
  STStatsEntry io_read;
  STStatsEntry io_write;
  STStatsEntry priority;
  STStatsEntry threads;
  STStatsEntry icswitch;
  STStatsEntry cpu;

  self = ST_PROCESS_STATS_COLLECTOR (obj);
  priv = ST_PROCESS_STATS_COLLECTOR_GET_PRIVATE (self);

  st_stats_get_full (stats,
		     NULL,
		     &vmsize,
		     &vmrss,
		     &vmswap,
		     &io_read,
		     &io_write, &priority, &threads, &icswitch, &cpu);


  gulong utime_curr, stime_curr, nivcsw_curr;
  long priority_curr, num_threads_curr;

  gint64 monotonic_time_curr = g_get_monotonic_time ();

  if (read_proc_status_memory (priv))
    {
      st_entry_update (&vmsize, priv->vmsize_current_kb);
      st_entry_update (&vmrss, priv->vmrss_current_kb);
      st_entry_update (&vmswap, priv->vmswap_current_kb);
    }
  else
    LOGW ("error read proc memory");

  if (!read_proc_stat_process (priv,
			       &utime_curr, &stime_curr,
			       &priority_curr, &num_threads_curr,
			       &nivcsw_curr))
    {
      LOGW ("error read stats");
      return FALSE;
    }

  gulong rchar_curr, wchar_curr;
  if (read_proc_io (priv, &rchar_curr, &wchar_curr))
    {
      gulong delta_rchar = rchar_curr - priv->rchar_prev;
      gulong delta_wchar = wchar_curr - priv->wchar_prev;

      double read_rate_kbs =
	(double) delta_rchar / 1024.0 / SAMPLING_INTERVAL_S;
      double write_rate_kbs =
	(double) delta_wchar / 1024.0 / SAMPLING_INTERVAL_S;

      st_entry_update (&io_read, read_rate_kbs);
      st_entry_update (&io_write, write_rate_kbs);

      priv->rchar_prev = rchar_curr;
      priv->wchar_prev = wchar_curr;
    }

  st_entry_update (&priority, priority_curr);
  st_entry_update (&threads, num_threads_curr);

  double context_switch_rate =
    calculate_context_switch_rate (priv, nivcsw_curr);

  st_entry_update (&icswitch, context_switch_rate);

  double current_cpu_usage =
    calculate_cpu_usage (priv, utime_curr, stime_curr, monotonic_time_curr);

  st_entry_update (&cpu, current_cpu_usage);

  priv->monotonic_time_prev = monotonic_time_curr;

  st_stats_set_full (stats,
		     NULL,
		     &vmsize,
		     &vmrss,
		     &vmswap,
		     &io_read,
		     &io_write, &priority, &threads, &icswitch, &cpu);

  return TRUE;
}

static void
st_process_stats_collector_finalize (GObject * object)
{
  LOGD ("finalize STProcessStatsCollector %p", object);

  G_OBJECT_CLASS (st_process_stats_collector_parent_class)->finalize (object);
}

static void
st_process_stats_collector_class_init (STProcessStatsCollectorClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  STStatsCollectorClass *collector_class = ST_STATS_COLLECTOR_CLASS (klass);

  gobject_class->finalize = st_process_stats_collector_finalize;
  collector_class->update = update_callback;
}

static void
st_process_stats_collector_init (STProcessStatsCollector * self)
{
  STProcessStatsCollectorPrivate *priv =
    st_process_stats_collector_get_instance_private (self);

  memset (priv, 0x00, sizeof (STProcessStatsCollectorPrivate));
}

STProcessStatsCollector *
st_process_stats_collector_new (STStats * stats, guint interval_ms)
{
  STProcessStatsCollector *r;
  STProcessStatsCollectorPrivate *priv;

  r = g_object_new (ST_TYPE_PROCESS_STATS_COLLECTOR,
		    "stats", stats, "interval", interval_ms, NULL);

  priv = ST_PROCESS_STATS_COLLECTOR_GET_PRIVATE (r);

  init_monitoring (priv);

  return r;
}
