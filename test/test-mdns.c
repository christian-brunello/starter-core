/*
 * starter core - test-mdns.c
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
#include <starter/mdns.h>
#include <starter/mdns-service.h>

#define TEST_SERVICE_NAME "Starter Test Service"
#define TEST_SERVICE_TYPE "_starter-test._tcp"
#define TEST_SERVICE_PORT 8080
#define TEST_SERVICE_HOST "starter-test.local"

typedef struct {
    GMainLoop *loop;
    STMDNS *mdns_publisher;
    STMDNS *mdns_browser;
    gboolean service_added_fired;
    gboolean service_removed_fired;
} TestContext;

/* Safety timeout callback to prevent the test from hanging if Avahi fails */
static gboolean
on_test_timeout (gpointer user_data)
{
    TestContext *ctx = (TestContext *) user_data;
    g_printerr ("Timeout reached! mDNS test components stalled.\n");
    g_main_loop_quit (ctx->loop);
    return G_SOURCE_REMOVE;
}

/* Callback triggered when a service is discovered ("service-added") */
static void
on_service_added (STMDNS *mdns, STMDNSService *service, gpointer user_data)
{
    TestContext *ctx = (TestContext *) user_data;
    const gchar *name = st_mdns_service_get_name (service);
    const gchar *type = st_mdns_service_get_mdns_type (service);

    // Filter discovered items so we only track our specific test payload
    if (g_strcmp0 (name, TEST_SERVICE_NAME) == 0 && g_strcmp0 (type, TEST_SERVICE_TYPE) == 0)
    {
        ctx->service_added_fired = TRUE;
        
        /* Step 2: Test removal. Now that it is discovered, we unref the publisher
         * to force Avahi to broadcast the goodbye packets on the local loopback.
         */
        g_clear_object (&ctx->mdns_publisher);
    }
}

/* Callback triggered when a service disappears ("service-removed") */
static void
on_service_removed (STMDNS *mdns, const char *name, gpointer user_data)
{
    TestContext *ctx = (TestContext *) user_data;

    if (g_strcmp0 (name, TEST_SERVICE_NAME) == 0)
    {
        ctx->service_removed_fired = TRUE;
        
        // Both conditions met successfully: stop the main loop execution cleanly
        g_main_loop_quit (ctx->loop);
    }
}

typedef struct {
    gint placeholder;
} TestFixture;

static void
fixture_setup (TestFixture *fixture, gconstpointer user_data)
{
}

static void
fixture_teardown (TestFixture *fixture, gconstpointer user_data)
{
}

/* Integrated Test: Register service through publisher instance, detect via browser signals */
static void
test_mdns_publish_and_browse (TestFixture *fixture, gconstpointer user_data)
{
    TestContext ctx;
    ctx.loop = g_main_loop_new (NULL, FALSE);
    ctx.service_added_fired = FALSE;
    ctx.service_removed_fired = FALSE;

    // Instantiate separate objects for cleaner architectural separation of roles
    ctx.mdns_publisher = st_mdns_new_with_options (0);
    ctx.mdns_browser = st_mdns_new_with_options (0);

    g_assert_nonnull (ctx.mdns_publisher);
    g_assert_nonnull (ctx.mdns_browser);

    // Connect the correct signals to our browser instance
    g_signal_connect (ctx.mdns_browser, "service-added", G_CALLBACK (on_service_added), &ctx);
    g_signal_connect (ctx.mdns_browser, "service-removed", G_CALLBACK (on_service_removed), &ctx);

    // Publish the test record service through the publisher instance
    gboolean published = st_mdns_publish_service (ctx.mdns_publisher,
                                                  TEST_SERVICE_NAME,
                                                  TEST_SERVICE_TYPE,
                                                  TEST_SERVICE_PORT);
    
    g_assert_true (published);

    // Inject a 5-second max execution limit source to guarantee clean execution builds
    g_timeout_add (5000, on_test_timeout, &ctx);

    // Execute the loop to trigger internal avahi-glib polls and the service-added signal
    g_main_loop_run (ctx.loop);

    /* --- Validation Assertions --- */
    g_assert_true (ctx.service_added_fired);
    g_assert_true (ctx.service_removed_fired);

    // Teardown remaining test context items
    g_main_loop_unref (ctx.loop);
    g_clear_object (&ctx.mdns_browser);
    if (ctx.mdns_publisher)
        g_clear_object (&ctx.mdns_publisher);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/libstarter-core/mdns/publish-and-browse",
                TestFixture, NULL,
                fixture_setup, test_mdns_publish_and_browse, fixture_teardown);

    return g_test_run ();
}

