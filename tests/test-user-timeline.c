#include <stdio.h>
#include <stdlib.h>
#include <glib-object.h>
#include <twitter-glib/twitter-glib.h>

static GMainLoop *main_loop = NULL;

static gboolean
authenticate_cb (TwitterClient    *client,
                 TwitterAuthState  state,
                 gpointer          user_data)
{
  gchar *user = NULL;

  twitter_client_get_user (client, &user, NULL);

  switch (state)
    {
    case TWITTER_AUTH_NEGOTIATING:
      g_print ("*** Authenticating as '%s'\n", user);
      break;

    case TWITTER_AUTH_SUCCESS:
      g_print ("*** Authentication successful\n");
      break;

    default:
      break;
    }

  g_free (user);

  return TRUE;
}

static void
status_received_cb (TwitterClient *client,
                    TwitterStatus *status,
                    const GError  *error,
                    gpointer       user_data)
{
  TwitterUser *user;

  if (error)
    {
      g_print ("Unable to retrieve the user timeline: %s\n", error->message);

      if (g_main_loop_is_running (main_loop))
        g_main_loop_quit (main_loop);

      return;
    }

  g_assert (TWITTER_IS_STATUS (status));

  user = twitter_status_get_user (status);
  if (!user)
    return;

  g_print ("<%s> %s\n"
           "\t-- created at: %s\n",
           twitter_user_get_name (user),
           twitter_status_get_text (status),
           twitter_status_get_created_at (status));
}

static void
timeline_received_cb (TwitterClient *client,
                      gpointer       user_data)
{
  if (g_main_loop_is_running (main_loop))
    g_main_loop_quit (main_loop);
}

static gboolean
get_user_timeline (gpointer data)
{
  TwitterClient *client = data;

  twitter_client_get_user_timeline (client, NULL, 0, 0);

  return FALSE;
}

static gboolean
timeout_quit (gpointer data)
{
  if (g_main_loop_is_running (main_loop))
    g_main_loop_quit (main_loop);

  return FALSE;
}

int
main (int   argc,
      char *argv[])
{
  TwitterClient *client;

  g_type_init ();
  g_thread_init (NULL);

  if (argc < 3)
    {
      g_print ("Usage: test-user-timeline <email> <password>\n");
      return EXIT_FAILURE;
    }

  client = twitter_client_new_for_user (argv[1], argv[2]);
  g_signal_connect (client, "authenticate",
                    G_CALLBACK (authenticate_cb),
                    NULL);
  g_signal_connect (client, "status-received",
                    G_CALLBACK (status_received_cb),
                    NULL);
  g_signal_connect (client, "timeline-complete",
                    G_CALLBACK (timeline_received_cb),
                    NULL);

  main_loop = g_main_loop_new (NULL, FALSE);

  g_timeout_add_seconds (1, get_user_timeline, client);
  g_timeout_add_seconds (10, timeout_quit, NULL);

  g_main_loop_run (main_loop);

  g_object_unref (client);
  g_main_loop_unref (main_loop);

  return EXIT_SUCCESS;
}
