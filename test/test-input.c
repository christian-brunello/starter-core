/*
 * starter core - test-input.c
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
#include <starter/input.h>
#include <starter/error.h>

typedef struct {
    guint changed_count;
    guint val_changed_count;
    guint min_changed_count;
    guint max_changed_count;
    guint step_changed_count;
} SignalTracker;

static void
on_changed (STInput *input, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->changed_count++;
}

static void
on_val_changed (STInput *input, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->val_changed_count++;
}

static void
on_min_changed (STInput *input, gpointer user_data)
{
    SignalTracker *tracker = (SignalTracker *) user_data;
    tracker->min_changed_count++;
}

typedef struct {
    STInput *input;
    SignalTracker tracker;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
    fixture->input = st_input_new ("test-sensor", 
                                  "Mock Temperature Sensor", 
                                  0,      /* STUnit placeholder */
                                  0.0,    /* min */
                                  100.0,  /* max */
                                  0.5,    /* step */
                                  25.0,   /* val */
                                  0);     /* flags */

    memset (&fixture->tracker, 0, sizeof (SignalTracker));

    g_signal_connect (fixture->input, "changed", G_CALLBACK (on_changed), &fixture->tracker);
    g_signal_connect (fixture->input, "val-changed", G_CALLBACK (on_val_changed), &fixture->tracker);
    g_signal_connect (fixture->input, "min-changed", G_CALLBACK (on_min_changed), &fixture->tracker);
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
    g_clear_object (&fixture->input);
}

/* Test 1: Verify initialization properties match inputs */
static void
test_input_initialization (TestFixture *fixture, gconstpointer user_data)
{
    g_assert_cmpstr (st_input_get_name (fixture->input), ==, "test-sensor");
    g_assert_cmpstr (st_input_get_description (fixture->input), ==, "Mock Temperature Sensor");
    g_assert_cmpfloat_with_epsilon (st_input_get_min (fixture->input), 0.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_input_get_max (fixture->input), 100.0, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_input_get_step (fixture->input), 0.5, 1e-9);
    g_assert_cmpfloat_with_epsilon (st_input_get_val (fixture->input), 25.0, 1e-9);
}

/* Test 2: Verify valid vs invalid value sets and proper GError generation */
static void
test_input_set_value_boundaries (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    // Set a valid value divisible by step (0.5)
    success = st_input_set_val (fixture->input, 26.5, &error);
    g_assert_true (success);
    g_assert_no_error (error);
    g_assert_cmpfloat_with_epsilon (st_input_get_val (fixture->input), 26.5, 1e-9);
    
    // Track if signals were successfully propagated
    g_assert_cmpint (fixture->tracker.val_changed_count, ==, 1);
    g_assert_cmpint (fixture->tracker.changed_count, ==, 1);

    // Set an invalid value violating maximum range constraint
    success = st_input_set_val (fixture->input, 105.0, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);

    // Set an invalid value violating the step constraint (26.3 is not divisible by 0.5)
    success = st_input_set_val (fixture->input, 26.3, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);
}

/* Test 3: Verify precision handling of floating-point boundaries via ismul() */
static void
test_input_step_and_precision (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    // Changing step to 0.1 should succeed since 0.0, 100.0, and 25.0 are multiples of 0.1
    success = st_input_set_step (fixture->input, 0.1, &error);
    g_assert_true (success);
    g_assert_no_error (error);

    // Changing step to 3.0 should fail because current val (25.0) is not a multiple of 3.0
    success = st_input_set_step (fixture->input, 3.0, &error);
    g_assert_false (success);
    g_assert_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE);
    g_clear_error (&error);
}

/* Test 4: Ensure changing values to identical targets skips signal emission */
static void
test_input_no_redundant_signals (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    
    // Setting identical val (25.0) should not fire change notifications
    st_input_set_val (fixture->input, 25.0, &error);
    g_assert_no_error (error);
    g_assert_cmpint (fixture->tracker.val_changed_count, ==, 0);
    g_assert_cmpint (fixture->tracker.changed_count, ==, 0);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/input/initialization",
                TestFixture, NULL,
                fixture_setup, test_input_initialization, fixture_teardown);

    g_test_add ("/libstarter-core/input/set-value-boundaries",
                TestFixture, NULL,
                fixture_setup, test_input_set_value_boundaries, fixture_teardown);

    g_test_add ("/libstarter-core/input/step-and-precision",
                TestFixture, NULL,
                fixture_setup, test_input_step_and_precision, fixture_teardown);

    g_test_add ("/libstarter-core/input/no-redundant-signals",
                TestFixture, NULL,
                fixture_setup, test_input_no_redundant_signals, fixture_teardown);

    return g_test_run ();
}

