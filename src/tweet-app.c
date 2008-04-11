#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include "tweet-app.h"

static TweetApp *default_app = NULL;

G_DEFINE_TYPE (TweetApp, tweet_app, G_TYPE_OBJECT);

static void
tweet_app_class_init (TweetAppClass *klass)
{
}

static void
tweet_app_init (TweetApp *app)
{
}

TweetApp *
tweet_app_get_default (int      *argc,
                       char   ***argv,
                       GError  **error)
{
  if (!default_app)
    {
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
  g_return_val_if_fail (tweet_app_is_running (app), EXIT_SUCCESS);

  gtk_main ();

  return EXIT_SUCCESS;
}
