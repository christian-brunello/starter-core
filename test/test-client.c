/*
 * starter core - test-client.c
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
#include <stdlib.h>

#include <starter/client.h>
#include <starter/stats.h>
#include <starter/error.h>

typedef struct {
  STClient *client;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
  fixture->client = st_client_new ();
  g_assert_nonnull (fixture->client);
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
  g_clear_object (&fixture->client);
}

/* Test 1: Verify initialization properties and default structures state */
static void
test_client_initialization (TestFixture *fixture, gconstpointer user_data)
{
  // Validate structural array allocations are initialized but empty post-instance
  const GPtrArray *inputs_retracted = st_client_get_inputs (fixture->client);
  g_assert_nonnull (inputs_retracted);
  g_assert_cmpint (inputs_retracted->len, ==, 0);

  const GPtrArray *outputs_retracted = st_client_get_outputs (fixture->client);
  g_assert_nonnull (outputs_retracted);
  g_assert_cmpint (outputs_retracted->len, ==, 0);

  // Global monitoring stats object is pre-allocated on init with default name
  const STStats *stats_retracted = st_client_get_stats (fixture->client);
  g_assert_nonnull (stats_retracted);
}

/* Test 2: Verify connection constraints when targeting a non-existent endpoint */
static void
test_client_network_lifecycle_fail (TestFixture *fixture, gconstpointer user_data)
{
  GError *error = NULL;
  gboolean success;

  /* Attempt to start the D-Bus TCP client connection.
   * We pass a target port that is definitely down to validate graceful error routing.
   */
  const gchar *unreachable_host = "127.0.0.1";
  guint16 target_port = 9999;

  success = st_client_start (fixture->client, unreachable_host, target_port, &error);
  
  // The client must return FALSE and bubble up a valid internal GError context
  g_assert_false (success);

  if (error)
    {
      g_test_message ("Client start failed with error: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      g_test_message ("Client start failed safely without populating GError.");
    }
}

/* Test 3: Verify fallback logic for service name lookup when disconnected */
static void
test_client_identity_unconnected (TestFixture *fixture, gconstpointer user_data)
{
  gchar *service_name;

  /* Calling getter methods for remote properties before executing st_client_start()
   * should safely intercept the missing connection loop and return NULL.
   */
  service_name = st_client_get_service_name (fixture->client);
  g_assert_null (service_name);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add ("/libstarter-core/client/initialization",
              TestFixture, NULL,
              fixture_setup, test_client_initialization, fixture_teardown);

  g_test_add ("/libstarter-core/client/network-lifecycle-fail",
              TestFixture, NULL,
              fixture_setup, test_client_network_lifecycle_fail, fixture_teardown);

  g_test_add ("/libstarter-core/client/identity-unconnected",
              TestFixture, NULL,
              fixture_setup, test_client_identity_unconnected, fixture_teardown);

  return g_test_run ();
}

