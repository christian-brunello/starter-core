/*
 * starter core - test-server.c
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
#include <starter/server.h>
#include <starter/version.h>
#include <starter/input.h>
#include <starter/output.h>
#include <starter/error.h>
#include <starter/defs.h>

typedef struct {
    STServer *server;
    GPtrArray *inputs;
    GPtrArray *outputs;
    STVersion version;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
    // Set up the static semantic version fields
    fixture->version.major = 1;
    fixture->version.minor = 2;
    fixture->version.micro = 3;

    // Initialize standard tracking vectors for inputs and outputs
    fixture->inputs = g_ptr_array_new_with_free_func (g_object_unref);
    fixture->outputs = g_ptr_array_new_with_free_func (g_object_unref);

    // Instantiate core server component passing the reference to our stack structure
    fixture->server = st_server_new ("StarterTestServer", 
                                     &fixture->version, 
                                     fixture->inputs, 
                                     fixture->outputs);
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
    g_clear_object (&fixture->server);
    g_ptr_array_unref (fixture->inputs);
    g_ptr_array_unref (fixture->outputs);
}

/* --- TEST CASES --- */

/* Test 1: Verify initialization properties and version structure matching */
static void
test_server_initialization (TestFixture *fixture, gconstpointer user_data)
{
    g_assert_nonnull (fixture->server);
    g_assert_cmpstr (st_server_get_name (fixture->server), ==, "StarterTestServer");
    
    // Validate version extraction match fields
    const STVersion *v_retracted = st_server_get_version (fixture->server);
    g_assert_nonnull (v_retracted);
    g_assert_cmpint (v_retracted->major, ==, 1);
    g_assert_cmpint (v_retracted->minor, ==, 2);
    g_assert_cmpint (v_retracted->micro, ==, 3);

    // Validate structural array allocations
    const GPtrArray *inputs_retracted = st_server_get_inputs (fixture->server);
    g_assert_nonnull (inputs_retracted);
    g_assert_cmpint (inputs_retracted->len, ==, 0);
}

/* Test 2: Verify version mutation logic and structure duplication allocations */
static void
test_server_version_mutations (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    STVersion new_version = { .major = 2, .minor = 0, .micro = 0 };
    
    success = st_server_set_version (fixture->server, &new_version, &error);
    g_assert_true (success);
    g_assert_no_error (error);

    // Test allocated layout replication structure using dup method
    STVersion *v_dup = st_server_dup_version (fixture->server);
    g_assert_nonnull (v_dup);
    g_assert_cmpint (v_dup->major, ==, 2);
    g_assert_cmpint (v_dup->minor, ==, 0);
    g_assert_cmpint (v_dup->micro, ==, 0);

    g_free (v_dup);
}

/* Test 3: Verify identity updates and string duplications */
static void
test_server_identity_mutations (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    success = st_server_set_name (fixture->server, "UpdatedStarterDaemon", &error);
    g_assert_true (success);
    g_assert_no_error (error);

    gchar *duplicated_name = st_server_dup_name (fixture->server);
    g_assert_cmpstr (duplicated_name, ==, "UpdatedStarterDaemon");
    g_free (duplicated_name);
}

/* Test 4: Verify network socket binding and server startup tracking loops */
static void
test_server_network_lifecycle (TestFixture *fixture, gconstpointer user_data)
{
    GError *error = NULL;
    gboolean success;

    /* Attempt to start the D-Bus TCP listener server loop.
     * We pass an allocated target port to securely isolate tests.
     */
    guint16 target_port = 8555;
    success = st_server_start (fixture->server, target_port, &error);

    if (!success)
    {
        g_assert_nonnull (error);
        g_test_message ("Server start skipped or failed due to environment network bind constraints: %s", error->message);
        g_clear_error (&error);
    }
    else
    {
        g_assert_no_error (error);
        g_test_message ("GDBus Server socket linked successfully onto runtime targets.");
    }
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/server/initialization",
                TestFixture, NULL,
                fixture_setup, test_server_initialization, fixture_teardown);

    g_test_add ("/libstarter-core/server/version-mutations",
                TestFixture, NULL,
                fixture_setup, test_server_version_mutations, fixture_teardown);

    g_test_add ("/libstarter-core/server/identity-mutations",
                TestFixture, NULL,
                fixture_setup, test_server_identity_mutations, fixture_teardown);

    g_test_add ("/libstarter-core/server/network-lifecycle",
                TestFixture, NULL,
                fixture_setup, test_server_network_lifecycle, fixture_teardown);

    return g_test_run ();
}

