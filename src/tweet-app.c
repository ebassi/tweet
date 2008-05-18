#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter-gtk/gtk-clutter-embed.h>

#include "tweet-app.h"
#include "tweet-config.h"
#include "tweet-window.h"

static TweetApp *default_app = NULL;

G_DEFINE_TYPE (TweetApp, tweet_app, G_TYPE_OBJECT);

static void
on_window_destroy (GtkWidget *widget,
                   TweetApp  *app)
{
  app->main_window = NULL;

  tweet_config_save (app->config);

  gtk_main_quit ();
}

static void
tweet_app_class_init (TweetAppClass *klass)
{
}

static void
tweet_app_init (TweetApp *app)
{
  app->config = tweet_config_get_default ();

  app->main_window = tweet_window_new ();
  g_signal_connect (app->main_window, "destroy",
                    G_CALLBACK (on_window_destroy),
                    app);
}

TweetApp *
tweet_app_get_default (int      *argc,
                       char   ***argv,
                       GError  **error)
{
  if (!default_app)
    {
      g_thread_init (NULL);
      gtk_init (argc, argv);
      clutter_init (argc, argv);

      default_app = g_object_new (TWEET_TYPE_APP, NULL);
    }

  return default_app;
}

gboolean
tweet_app_is_running (TweetApp *app)
{
  g_return_val_if_fail (TWEET_IS_APP (app), FALSE);

  return app->is_running;
}

gint
tweet_app_run (TweetApp *app)
{
  g_return_val_if_fail (TWEET_IS_APP (app), EXIT_FAILURE);
  g_return_val_if_fail (!tweet_app_is_running (app), EXIT_SUCCESS);

  if (!tweet_config_get_username (app->config))
    {
      /* ask the user for the credentials */
    }

  gtk_main ();

  tweet_config_save (app->config);

  return EXIT_SUCCESS;
}
