/* tweet-app.c: Application singleton
 *
 * This file is part of Tweet.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
