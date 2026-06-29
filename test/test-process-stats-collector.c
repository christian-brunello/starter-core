/*
 * starter core - test-process-stats-collector.c
 *
 * Authored by Gemini AI (Google) as a collaborative engineering effort.
 * * CRITICAL LIMITATION: Only this specific file is dedicated to the public 
 * domain under the terms of the Creative Commons Zero (CC0 1.0 Universal) license. 
 * This dedication does NOT apply to any other files, source code, or architecture 
 * within the STARTER framework, which remain strictly protected.
 *
 * You can copy, modify, distribute and perform the work in this file, even for 
 * commercial purposes, all without asking permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#include <glib.h>
#include <gio/gio.h>
#include <starter/stats.h>
#include <starter/stats-collector.h>
#include <starter/process-stats-collector.h>

typedef struct {
    GMainLoop *loop;
    STStats *stats;
    STProcessStatsCollector *collector;
    gboolean data_received;
} TestContext;

static gboolean
on_test_timeout (gpointer user_data)
{
    TestContext *ctx = (TestContext *) user_data;
    g_main_loop_quit (ctx->loop);
    return G_SOURCE_REMOVE;
}

static void
on_stats_changed (STStats *stats, gpointer user_data)
{
    TestContext *ctx = (TestContext *) user_data;
    ctx->data_received = TRUE;
    
    g_main_loop_quit (ctx->loop);
}

typedef struct {
    STStats *stats;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
    fixture->stats = st_stats_new ("test-process-metrics");
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
    g_clear_object (&fixture->stats);
}

static void
test_collector_automatic_cycle (TestFixture *fixture, gconstpointer user_data)
{
    TestContext ctx;
    ctx.loop = g_main_loop_new (NULL, FALSE);
    ctx.stats = fixture->stats;
    ctx.data_received = FALSE;

    g_signal_connect (ctx.stats, "changed", G_CALLBACK (on_stats_changed), &ctx);

    ctx.collector = st_process_stats_collector_new (ctx.stats, 500);
    g_assert_nonnull (ctx.collector);
    g_assert_true (ST_IS_STATS_COLLECTOR (ctx.collector));

    g_timeout_add (2000, on_test_timeout, &ctx);

    g_main_loop_run (ctx.loop);

    g_assert_true (ctx.data_received);

    const STStatsEntry *cpu = st_stats_get_cpu (ctx.stats);
    const STStatsEntry *vmrss = st_stats_get_vmrss (ctx.stats);
    const STStatsEntry *threads = st_stats_get_threads (ctx.stats);

    g_assert_cmpfloat (vmrss->last, >, 0.0);
    g_assert_cmpfloat (threads->last, >=, 1.0);
    g_assert_cmpfloat (cpu->last, >=, 0.0);

    g_main_loop_unref (ctx.loop);
    g_clear_object (&ctx.collector);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/process-stats-collector/automatic-cycle",
                TestFixture, NULL,
                fixture_setup, test_collector_automatic_cycle, fixture_teardown);

    return g_test_run ();
}

