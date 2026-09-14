/*
 * starter core - test-client-server-armageddon-async.c
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <starter/server.h>
#include <starter/client.h>
#include <starter/version.h>
#include <starter/input.h>
#include <starter/output.h>
#include <starter/stats.h>
#include <starter/error.h>

#define STRESS_ITERATIONS 200

static pid_t server_pid = -1;

typedef struct
{
  STClient *client;
  GMainLoop *loop;
  gboolean finished;
  gboolean success;
  GError *error;
} AsyncStartCtx;

static void
kill_server_process (void)
{
  if (server_pid > 0)
    {
      g_test_message ("Stopping the daemon post-armageddon-async (PID: %d)...",
		      server_pid);
      kill (server_pid, SIGTERM);
      waitpid (server_pid, NULL, 0);
      server_pid = -1;
    }
}

static gboolean
on_iter_timeout (gpointer user_data)
{
  AsyncStartCtx *ctx = user_data;

  g_test_message ("Timeout waiting for async client start in stress loop");
  g_main_loop_quit (ctx->loop);
  return G_SOURCE_REMOVE;
}

static void
on_start_finished (GObject * source, GAsyncResult * result, gpointer user_data)
{
  AsyncStartCtx *ctx = user_data;

  ctx->success =
    st_client_start_finish (ST_CLIENT (source), result, &ctx->error);
  ctx->finished = TRUE;
  g_main_loop_quit (ctx->loop);
}

static void
test_e2e_armageddon_async_stress (gconstpointer user_data)
{
  int *pipe_fds = (int *) user_data;
  guint16 assigned_port = 0;
  GMainLoop *loop;
  int i;

  close (pipe_fds[1]);

  if (read (pipe_fds[0], &assigned_port, sizeof (assigned_port)) !=
      sizeof (assigned_port))
    {
      g_error ("Armageddon-async failed early: Could not read port from server child!");
    }
  close (pipe_fds[0]);

  g_test_message
    ("Target acquired on port %hu. Starting async destructive stress test (%d iterations)...",
     assigned_port, STRESS_ITERATIONS);

  loop = g_main_loop_new (NULL, FALSE);

  for (i = 0; i < STRESS_ITERATIONS; i++)
    {
      AsyncStartCtx ctx = { 0 };
      guint timeout_id;
      gchar *fetched_name;
      const GPtrArray *inputs;
      const GPtrArray *outputs;

      ctx.client = st_client_new ();
      ctx.loop = loop;
      g_assert_nonnull (ctx.client);

      timeout_id = g_timeout_add_seconds (10, on_iter_timeout, &ctx);

      st_client_start_async (ctx.client, "127.0.0.1", assigned_port, NULL,
			     on_start_finished, &ctx);

      g_assert_cmpint (st_client_get_state (ctx.client), ==,
		       ST_CLIENT_STATE_PENDING);

      g_main_loop_run (loop);
      g_source_remove (timeout_id);

      g_assert_true (ctx.finished);
      g_assert_true (ctx.success);
      g_assert_no_error (ctx.error);
      g_assert_cmpint (st_client_get_state (ctx.client), ==,
		       ST_CLIENT_STATE_READY);
      g_assert_true (st_client_is_ready (ctx.client));

      fetched_name = st_client_get_service_name (ctx.client);
      g_assert_nonnull (fetched_name);
      g_assert_cmpstr (fetched_name, ==, "StarterIntegrationDaemon");
      g_free (fetched_name);

      inputs = st_client_get_inputs (ctx.client);
      g_assert_nonnull (inputs);
      g_assert_cmpint (inputs->len, ==, 1);

      outputs = st_client_get_outputs (ctx.client);
      g_assert_nonnull (outputs);
      g_assert_cmpint (outputs->len, ==, 1);

      g_object_unref (ctx.client);

      if ((i + 1) % 50 == 0)
	{
	  g_test_message
	    ("Armageddon-async Progress: %d/%d cycles completed safely.",
	     i + 1, STRESS_ITERATIONS);
	}
    }

  g_main_loop_unref (loop);

  g_test_message
    ("The server survived the async Armageddon! Zero leaks or deadlocks detected across %d connection cycles.",
     STRESS_ITERATIONS);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  int pipe_fds[2];
  if (pipe (pipe_fds) == -1)
    {
      g_error ("Critical: Failed to allocate communication pipes.");
    }

  server_pid = fork ();
  if (server_pid < 0)
    {
      g_error ("Critical: Failed to execute fork.");
    }

  if (server_pid == 0)
    {
      GMainLoop *server_loop;
      STServer *server;
      GPtrArray *inputs;
      GPtrArray *outputs;
      STVersion version = { .major = 1, .minor = 0, .micro = 0 };
      GError *error = NULL;
      guint16 dynamic_port = 0;

      close (pipe_fds[0]);

      server_loop = g_main_loop_new (NULL, FALSE);

      inputs = g_ptr_array_new_with_free_func (g_object_unref);
      outputs = g_ptr_array_new_with_free_func (g_object_unref);

      g_ptr_array_add (inputs,
		       st_input_new ("TestInput", "TestInput", ST_UNIT_NULL,
				     0, 100, 1, 1, 0));
      g_ptr_array_add (outputs,
		       st_output_new ("TestOutput", "TestOutput", ST_UNIT_NULL,
				      0, 100, 1, 1, 0));

      server =
	st_server_new ("StarterIntegrationDaemon", &version, inputs, outputs);
      g_assert_nonnull (server);

      if (!st_server_start (server, 0, &error))
	{
	  g_error ("Child server failed to bind: %s", error->message);
	}

      dynamic_port = st_server_get_port (server);

      if (write (pipe_fds[1], &dynamic_port, sizeof (dynamic_port)) !=
	  sizeof (dynamic_port))
	{
	  g_error ("Child server failed to pipe the allocated port back.");
	}
      close (pipe_fds[1]);

      g_main_loop_run (server_loop);

      g_object_unref (server);
      g_ptr_array_unref (inputs);
      g_ptr_array_unref (outputs);
      g_main_loop_unref (server_loop);

      _exit (EXIT_SUCCESS);
    }
  else
    {
      atexit (kill_server_process);

      g_test_add_data_func
	("/libstarter-core/integration/armageddon-stress-e2e-async",
	 pipe_fds, test_e2e_armageddon_async_stress);

      int test_result = g_test_run ();

      kill_server_process ();

      return test_result;
    }
}
