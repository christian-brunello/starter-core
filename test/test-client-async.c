/*
 * starter core - test-client-async.c
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

typedef struct
{
  STClient *client;
  GMainLoop *loop;
  gboolean finished;
  gboolean success;
  GError *error;
} TestFixture;

typedef struct
{
  gboolean finished;
  gboolean success;
  GError *error;
} SecondStartCtx;

static gboolean
on_test_timeout (gpointer user_data)
{
  TestFixture *fixture = user_data;

  g_test_message ("Timeout waiting for async client operation");
  g_main_loop_quit (fixture->loop);
  return G_SOURCE_REMOVE;
}

static void
fixture_setup (TestFixture * fixture, gconstpointer user_data)
{
  fixture->client = st_client_new ();
  fixture->loop = g_main_loop_new (NULL, FALSE);
  fixture->finished = FALSE;
  fixture->success = FALSE;
  fixture->error = NULL;

  g_assert_nonnull (fixture->client);
  g_assert_nonnull (fixture->loop);
}

static void
fixture_teardown (TestFixture * fixture, gconstpointer user_data)
{
  g_clear_error (&fixture->error);
  g_clear_object (&fixture->client);
  if (fixture->loop)
    g_main_loop_unref (fixture->loop);
}

static void
on_start_finished (GObject * source, GAsyncResult * result, gpointer user_data)
{
  TestFixture *fixture = user_data;

  fixture->success =
    st_client_start_finish (ST_CLIENT (source), result, &fixture->error);
  fixture->finished = TRUE;
  g_main_loop_quit (fixture->loop);
}

static void
on_second_start_finished (GObject * source, GAsyncResult * result,
			  gpointer user_data)
{
  SecondStartCtx *ctx = user_data;

  ctx->success =
    st_client_start_finish (ST_CLIENT (source), result, &ctx->error);
  ctx->finished = TRUE;
}

static void
test_client_async_state_idle (TestFixture * fixture, gconstpointer user_data)
{
  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_IDLE);
  g_assert_false (st_client_is_ready (fixture->client));

  g_assert_cmpint (st_client_get_inputs (fixture->client)->len, ==, 0);
  g_assert_cmpint (st_client_get_outputs (fixture->client)->len, ==, 0);
  g_assert_nonnull (st_client_get_stats (fixture->client));
}

static void
test_client_async_network_lifecycle_fail (TestFixture * fixture,
					  gconstpointer user_data)
{
  guint timeout_id;

  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_IDLE);

  timeout_id = g_timeout_add_seconds (5, on_test_timeout, fixture);

  st_client_start_async (fixture->client, "127.0.0.1", 9999, NULL,
			 on_start_finished, fixture);

  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_PENDING);

  g_main_loop_run (fixture->loop);
  g_source_remove (timeout_id);

  g_assert_true (fixture->finished);
  g_assert_false (fixture->success);
  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_FAILED);
  g_assert_false (st_client_is_ready (fixture->client));

  if (fixture->error)
    g_test_message ("Async start failed as expected: %s",
		    fixture->error->message);
}

static void
test_client_async_cancel_pending (TestFixture * fixture,
				  gconstpointer user_data)
{
  GCancellable *cancellable;
  guint timeout_id;

  cancellable = g_cancellable_new ();
  timeout_id = g_timeout_add_seconds (5, on_test_timeout, fixture);

  st_client_start_async (fixture->client, "127.0.0.1", 9999, cancellable,
			 on_start_finished, fixture);

  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_PENDING);

  g_cancellable_cancel (cancellable);

  g_main_loop_run (fixture->loop);
  g_source_remove (timeout_id);

  g_assert_true (fixture->finished);
  g_assert_false (fixture->success);
  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_FAILED);

  if (fixture->error)
    g_test_message ("Cancel path error: %s", fixture->error->message);

  g_object_unref (cancellable);
}

static void
test_client_async_double_start (TestFixture * fixture, gconstpointer user_data)
{
  SecondStartCtx second = { FALSE, FALSE, NULL };
  guint timeout_id;

  timeout_id = g_timeout_add_seconds (5, on_test_timeout, fixture);

  st_client_start_async (fixture->client, "127.0.0.1", 9999, NULL,
			 on_start_finished, fixture);
  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_PENDING);

  /* Second start while pending must fail immediately via the async result. */
  st_client_start_async (fixture->client, "127.0.0.1", 9999, NULL,
			 on_second_start_finished, &second);

  /* Allow the immediate error callback to be dispatched. */
  while (!second.finished && g_main_context_pending (NULL))
    g_main_context_iteration (NULL, FALSE);

  g_assert_true (second.finished);
  g_assert_false (second.success);
  g_assert_nonnull (second.error);
  g_test_message ("Double-start rejected: %s", second.error->message);
  g_clear_error (&second.error);

  g_main_loop_run (fixture->loop);
  g_source_remove (timeout_id);

  g_assert_true (fixture->finished);
  g_assert_false (fixture->success);
  g_assert_cmpint (st_client_get_state (fixture->client), ==,
		   ST_CLIENT_STATE_FAILED);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add ("/libstarter-core/client-async/state-idle",
	      TestFixture, NULL,
	      fixture_setup, test_client_async_state_idle, fixture_teardown);

  g_test_add ("/libstarter-core/client-async/network-lifecycle-fail",
	      TestFixture, NULL,
	      fixture_setup, test_client_async_network_lifecycle_fail,
	      fixture_teardown);

  g_test_add ("/libstarter-core/client-async/cancel-pending",
	      TestFixture, NULL,
	      fixture_setup, test_client_async_cancel_pending,
	      fixture_teardown);

  g_test_add ("/libstarter-core/client-async/double-start",
	      TestFixture, NULL,
	      fixture_setup, test_client_async_double_start, fixture_teardown);

  return g_test_run ();
}
