/*
 * starter core - test-stats.c
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
#include <math.h>
#include <starter/stats.h>

typedef struct {
    guint changed_signal_count;
} SignalTracker;

static void
on_stats_changed (STStats *stats, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->changed_signal_count++;
}

typedef struct {
    STStats *stats;
    SignalTracker tracker;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
    fixture->stats = st_stats_new ("starter-daemon-metrics");
    fixture->tracker.changed_signal_count = 0;

    g_signal_connect (fixture->stats, "changed", G_CALLBACK (on_stats_changed), &fixture->tracker);
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
    g_clear_object (&fixture->stats);
}

/* --- TEST CASES --- */

/* Test 1: Validate initialization bounds defined by st_entry_init() */
static void
test_stats_initialization (TestFixture *fixture, gconstpointer user_data)
{
    g_assert_cmpstr (st_stats_get_name (fixture->stats), ==, "starter-daemon-metrics");

    const STStatsEntry *cpu = st_stats_get_cpu (fixture->stats);
    const STStatsEntry *vmrss = st_stats_get_vmrss (fixture->stats);

    // st_entry_init forces min to G_MAXDOUBLE and max to G_MINDOUBLE
    g_assert_cmpfloat (cpu->min, ==, G_MAXDOUBLE);
    g_assert_cmpfloat (cpu->max, ==, G_MINDOUBLE);
    g_assert_cmpfloat_with_epsilon (cpu->last, 0.0, 1e-9);

    g_assert_cmpfloat (vmrss->min, ==, G_MAXDOUBLE);
    g_assert_cmpfloat (vmrss->max, ==, G_MINDOUBLE);
}

/* Test 2: Validate entry metrics tracking limits (Min / Max / Last logic) */
static void
test_stats_entry_updates (TestFixture *fixture, gconstpointer user_data)
{
    STStatsEntry mock_entry;
    st_entry_init (&mock_entry);

    // First update pass
    st_entry_update (&mock_entry, 45.5);
    g_assert_cmpfloat_with_epsilon (mock_entry.min, 45.5, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.max, 45.5, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.last, 45.5, 1e-9);

    // Second update pass with lower bound values
    st_entry_update (&mock_entry, 12.0);
    g_assert_cmpfloat_with_epsilon (mock_entry.min, 12.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.max, 45.5, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.last, 12.0, 1e-9);

    // Third update pass with higher bound values
    st_entry_update (&mock_entry, 88.2);
    g_assert_cmpfloat_with_epsilon (mock_entry.min, 12.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.max, 88.2, 1e-9);
    g_assert_cmpfloat_with_epsilon (mock_entry.last, 88.2, 1e-9);
}

/* Test 3: Validate threshold-based signal emission triggers (entry_diff logic) */
static void
test_stats_signal_thresholds (TestFixture *fixture, gconstpointer user_data)
{
    STStatsEntry sample;
    sample.min = 10.0;
    sample.max = 50.0;
    sample.last = 30.0;

    // Setting values initially will alter state past threshold limit (1.0 default)
    st_stats_set_cpu (fixture->stats, &sample);
    g_assert_cmpint (fixture->tracker.changed_signal_count, ==, 1);

    // Sub-threshold updates shouldn't emit signals (delta 0.5 < THRESHOLD 1.0)
    sample.last = 30.5;
    st_stats_set_cpu (fixture->stats, &sample);
    g_assert_cmpint (fixture->tracker.changed_signal_count, ==, 1);

    // Out-of-bounds update exceeding or meeting threshold (delta 1.5 >= THRESHOLD 1.0)
    sample.last = 32.0;
    st_stats_set_cpu (fixture->stats, &sample);
    g_assert_cmpint (fixture->tracker.changed_signal_count, ==, 2);
}

/* Test 4: Verify full structure copy operations and block updates */
static void
test_stats_duplication_and_bulk (TestFixture *fixture, gconstpointer user_data)
{
    STStatsEntry item = { .min = 5.0, .max = 25.0, .last = 15.0 };
    st_stats_set_vmsize (fixture->stats, &item);

    // Verify duplication clone matches layout
    STStats *cloned = st_stats_dup (fixture->stats);
    g_assert_cmpstr (st_stats_get_name (cloned), ==, "starter-daemon-metrics");
    
    const STStatsEntry *cloned_vmsize = st_stats_get_vmsize (cloned);
    g_assert_cmpfloat_with_epsilon (cloned_vmsize->last, 15.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (cloned_vmsize->max, 25.0, 1e-9);

    // Test bulk retrieval engine
    STStatsEntry retrieved_vmsize;
    const gchar *name_out = NULL;
    
    st_stats_get_full (fixture->stats, &name_out, &retrieved_vmsize, 
                      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    g_assert_cmpstr (name_out, ==, "starter-daemon-metrics");
    g_assert_cmpfloat_with_epsilon (retrieved_vmsize.last, 15.0, 1e-9);

    g_object_unref (cloned);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/stats/initialization",
                TestFixture, NULL,
                fixture_setup, test_stats_initialization, fixture_teardown);

    g_test_add ("/libstarter-core/stats/entry-updates",
                TestFixture, NULL,
                fixture_setup, test_stats_entry_updates, fixture_teardown);

    g_test_add ("/libstarter-core/stats/signal-thresholds",
                TestFixture, NULL,
                fixture_setup, test_stats_signal_thresholds, fixture_teardown);

    g_test_add ("/libstarter-core/stats/duplication-and-bulk",
                TestFixture, NULL,
                fixture_setup, test_stats_duplication_and_bulk, fixture_teardown);

    return g_test_run ();
}

