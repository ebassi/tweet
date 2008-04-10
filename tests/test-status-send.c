#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <twitter-glib/twitter-glib.h>

static GMainLoop *main_loop = NULL;

static gboolean
authenticate_cb (TwitterClient    *client,
                 TwitterAuthState  state,
                 gpointer          user_data)
{
  g_print ("Authenticating...\n");

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
      g_print ("Unable to send the status: %s\n", error->message);

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

typedef struct {
  gchar *text;
  TwitterClient *client;
} SendStatusClosure;

static gboolean
send_status (gpointer data)
{
  SendStatusClosure *closure = data;

  twitter_client_add_status (closure->client, closure->text);

  g_free (closure->text);
  g_free (closure);

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
  SendStatusClosure *closure;

  g_type_init ();
  g_thread_init (NULL);

  if (argc < 4)
    {
      g_print ("Usage: test-status-send <email> <password> <text>\n");
      return EXIT_FAILURE;
    }

  client = twitter_client_new_for_user (argv[1], argv[2]);
  g_signal_connect (client, "authenticate",
                    G_CALLBACK (authenticate_cb),
                    NULL);
  g_signal_connect (client, "status-received",
                    G_CALLBACK (status_received_cb),
                    NULL);

  main_loop = g_main_loop_new (NULL, FALSE);

  closure = g_new (SendStatusClosure, 1);
  closure->client = client;
  closure->text = g_strdup (argv[3]);

  g_timeout_add_seconds (1, send_status, closure);
  g_timeout_add_seconds (10, timeout_quit, NULL);

  g_main_loop_run (main_loop);

  g_object_unref (client);
  g_main_loop_unref (main_loop);

  return EXIT_SUCCESS;
}
