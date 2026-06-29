/*
 * starter core - test-client-server.c
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

static pid_t server_pid = -1;

static void
kill_server_process (void)
{
  if (server_pid > 0)
    {
      g_test_message ("Terminating background server process (PID: %d)...", server_pid);
      kill (server_pid, SIGTERM);
      waitpid (server_pid, NULL, 0);
      server_pid = -1;
    }
}

static void
test_e2e_complete_pipeline (gconstpointer user_data)
{
  int *pipe_fds = (int *) user_data;
  GError *error = NULL;
  gboolean success;
  guint16 assigned_port = 0;
  STClient *client;

  close (pipe_fds[1]);

  if (read (pipe_fds[0], &assigned_port, sizeof (assigned_port)) != sizeof (assigned_port))
    {
      g_error ("Failed to read the dynamic port from the server child process!");
    }
  close (pipe_fds[0]);

  g_test_message ("Parent client targeting isolated port: %hu", assigned_port);

  client = st_client_new ();
  g_assert_nonnull (client);

  /* 1. NETWORK LIFECYCLE: Connect to the isolated background server process */
  success = st_client_start (client, "127.0.0.1", assigned_port, &error);
  g_assert_true (success);
  g_assert_no_error (error);

  /* 2. IDENTITY SYNC: Verify remote string property replication over D-Bus */
  gchar *fetched_name = st_client_get_service_name (client);
  g_assert_nonnull (fetched_name);
  g_assert_cmpstr (fetched_name, ==, "StarterIntegrationDaemon");
  g_free (fetched_name);

  /* 3. ARRAYS SYNC: Verify tracking vectors are populated upon connection */
  const GPtrArray *client_inputs = st_client_get_inputs (client);
  g_assert_nonnull (client_inputs);
  g_assert_cmpint (client_inputs->len, ==, 1);
  g_test_message ("Client successfully synchronized %u tracking input(s) from server.", client_inputs->len);

  const GPtrArray *client_outputs = st_client_get_outputs (client);
  g_assert_nonnull (client_outputs);
  g_assert_cmpint (client_outputs->len, ==, 1);
  g_test_message ("Client successfully synchronized %u tracking output(s) from server.", client_outputs->len);

  /* 4. MONITORING STATS: Verify global telemetry structure instantiation */
  const STStats *client_stats = st_client_get_stats (client);
  g_assert_nonnull (client_stats);
  g_test_message ("Client monitoring stats structure validated post-connection.");

  g_test_message ("Complete E2E validation pipeline executed successfully.");
  g_object_unref (client);
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
      g_error ("Critical: Failed to execute fork targeting separate processes.");
    }

  if (server_pid == 0)
    {
      /* ----------------------------------------------------
       * CHILD PROCESS: STServer Daemon Loop
       * ---------------------------------------------------- */
      GMainLoop *server_loop;
      STServer *server;
      GPtrArray *inputs;
      GPtrArray *outputs;
      STVersion version = { .major = 1, .minor = 0, .micro = 0 };
      GError *error = NULL;
      guint16 dynamic_port = 0;

      // Close unused read end of the pipe
      close (pipe_fds[0]);

      server_loop = g_main_loop_new (NULL, FALSE);

      inputs = g_ptr_array_new_with_free_func (g_object_unref);
      outputs = g_ptr_array_new_with_free_func (g_object_unref);

      gpointer mock_input = st_input_new("TestInput", "TestInput", ST_UNIT_NULL, 0, 100, 1, 1, 0);
      gpointer mock_output = st_output_new("TestOutput", "TestOutput", ST_UNIT_NULL, 0, 100, 1, 1, 0);

      g_ptr_array_add (inputs, mock_input);
      g_ptr_array_add (outputs, mock_output);

      // Instantiate the real server component
      server = st_server_new ("StarterIntegrationDaemon", &version, inputs, outputs);
      g_assert_nonnull (server);
      
      // Start server on dynamic port 0
      if (!st_server_start (server, 0, &error))
        {
          g_error ("Child server failed to bind: %s", error->message);
        }

      // Extract the port dynamically assigned by the kernel
      dynamic_port = st_server_get_port (server);

      // Send the dynamic port code to the parent client process via pipe
      if (write (pipe_fds[1], &dynamic_port, sizeof (dynamic_port)) != sizeof (dynamic_port))
        {
          g_error ("Child server failed to pipe the allocated port back.");
        }
      close (pipe_fds[1]);

      // Run the server event context indefinitely until parent terminates us
      g_main_loop_run (server_loop);

      // Cleanup allocations
      g_object_unref (server);
      g_ptr_array_unref (inputs);
      g_ptr_array_unref (outputs);
      g_main_loop_unref (server_loop);
      
      _exit (EXIT_SUCCESS);
    }
  else
    {
      /* ----------------------------------------------------
       * PARENT PROCESS: GTest Suites (STClient Runner)
       * ---------------------------------------------------- */
      atexit (kill_server_process);

      g_test_add_data_func ("/libstarter-core/integration/complete-pipeline-e2e",
                            pipe_fds,
                            test_e2e_complete_pipeline);

      int test_result = g_test_run ();

      kill_server_process ();

      return test_result;
    }
}

