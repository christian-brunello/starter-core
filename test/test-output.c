/*
 * starter core - test-output.c
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
#include <starter/output.h>
#include <starter/error.h>

/* Helper structure to track signal emission counts */
typedef struct {
    guint changed_count;
    guint val_changed_count;
    guint min_changed_count;
    guint max_changed_count;
    guint step_changed_count;
} SignalTracker;

/* Signal callbacks to increment counters */
static void
on_changed (STOutput *output, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->changed_count++;
}

static void
on_val_changed (STOutput *output, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->val_changed_count++;
}

static void
on_min_changed (STOutput *output, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->min_changed_count++;
}

/* Define fixture to isolate state across individual test runs */
typedef struct {
    STOutput *output;
    SignalTracker tracker;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
    // Initialize component with standard mock properties:
    // min, max, val must be multiples of step (1.0)
    fixture->output = st_output_new ("test-relay", 
                                    "Mock Relay Output Control", 
                                    0,       /* STUnit placeholder */
                                    0.0,     /* min */
                                    250.0,   /* max */
                                    1.0,     /* step */
                                    0.0,     /* val */
                                    0);      /* flags */

    // Reset tracking indicators
    memset (&fixture->tracker, 0, sizeof (SignalTracker));

    // Connect signals for emission analysis
    g_signal_connect (fixture->output, "changed", G_CALLBACK (on_changed), &fixture->tracker);
    g_signal_connect (fixture->output, "val-changed", G_CALLBACK (on_val_changed), &fixture->tracker);
    g_signal_connect (fixture->output, "min-changed", G_CALLBACK (on_min_changed), &fixture->tracker);
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
    g_clear_object (&fixture->output);
}

/* --- TEST CASES --- */

/* Test 1: Verify initialization properties match inputs */
static void
test_output_initialization (TestFixture *fixture, gconstpointer user_data)
{
    g_assert_cmpstr (st_output_get_name (fixture->output), ==, "test-relay");
    g_assert_cmpstr (st_output_get_description (fixture->output), ==, "Mock Relay Output Control");
    g_assert_cmpfloat_with_epsilon (st_output_get_min (fixture->output), 0.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_output_get_max (fixture->output), 250.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_output_get_step (fixture->output), 1.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_output_get_val (fixture->output), 0.0, 1e-9);
}

/* Test 2: Verify valid vs invalid value sets and proper GError generation */
static void
test_output_set_value_boundaries (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    // Set a valid value divisible by step (1.0)
    success = st_output_set_val (fixture->output, 50.0, &error);
    g_assert_true (success);
    g_assert_no_error (error);
    g_assert_cmpfloat_with_epsilon (st_output_get_val (fixture->output), 50.0, 1e-9);
    
    // Track if signals were successfully propagated
    g_assert_cmpint (fixture->tracker.val_changed_count, ==, 1);
    g_assert_cmpint (fixture->tracker.changed_count, ==, 1);

    // Set an invalid value violating maximum range constraint (255.0 > 240.0)
    success = st_output_set_val (fixture->output, 255.0, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);

    // Set an invalid value violating the step constraint (50.5 is not divisible by 1.0)
    success = st_output_set_val (fixture->output, 50.5, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);
}

/* Test 3: Verify precision handling of floating-point boundaries via ismul() */
static void
test_output_step_and_precision (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    // Changing step to 0.5 should succeed since 0.0, 250.0, and 0.0 are multiples of 0.5
    success = st_output_set_step (fixture->output, 0.5, &error);
    g_assert_true (success);
    g_assert_no_error (error);

    // Change current value to 5.5 to prepare edge case for next step update
    success = st_output_set_val (fixture->output, 5.5, &error);
    g_assert_true (success);
    g_clear_error (&error);

    // Changing step to 2.0 should fail because current val (5.5) is not a multiple of 2.0
    success = st_output_set_step (fixture->output, 2.0, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);
}

/* Test 4: Ensure changing values to identical targets skips signal emission */
static void
test_output_no_redundant_signals (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    
    // Setting identical val (0.0) should not fire change notifications
    st_output_set_val (fixture->output, 0.0, &error);
    g_assert_no_error (error);
    g_assert_cmpint (fixture->tracker.val_changed_count, ==, 0);
    g_assert_cmpint (fixture->tracker.changed_count, ==, 0);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/output/initialization",
                TestFixture, NULL,
                fixture_setup, test_output_initialization, fixture_teardown);

    g_test_add ("/libstarter-core/output/set-value-boundaries",
                TestFixture, NULL,
                fixture_setup, test_output_set_value_boundaries, fixture_teardown);

    g_test_add ("/libstarter-core/output/step-and-precision",
                TestFixture, NULL,
                fixture_setup, test_output_step_and_precision, fixture_teardown);

    g_test_add ("/libstarter-core/output/no-redundant-signals",
                TestFixture, NULL,
                fixture_setup, test_output_no_redundant_signals, fixture_teardown);

    return g_test_run ();
}

