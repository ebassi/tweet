/* tweet-app.h: Application singleton
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
#ifndef __TWEET_APP_H__
#define __TWEET_APP_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "tweet-config.h"

G_BEGIN_DECLS

#define TWEET_TYPE_APP            (tweet_app_get_type ())
#define TWEET_APP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_APP, TweetApp))
#define TWEET_IS_APP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_APP))
#define TWEET_APP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_APP, TweetAppClass))
#define TWEET_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_APP))
#define TWEET_APP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_APP, TweetAppClass))

typedef struct _TweetApp        TweetApp;
typedef struct _TweetAppClass   TweetAppClass;

struct _TweetApp
{
  GObject parent_instance;

  TweetConfig *config;

  GtkWidget *main_window;

  guint is_running : 1;
};

struct _TweetAppClass
{
  GObjectClass parent_class;
};

GType tweet_app_get_type (void) G_GNUC_CONST;

TweetApp *tweet_app_get_default (int        *argc,
                                 char     ***argv,
                                 GError    **error);
gboolean  tweet_app_is_running  (TweetApp   *app);
gint      tweet_app_run         (TweetApp   *app);

G_END_DECLS

#endif /* __TWEET_APP_H__ */
