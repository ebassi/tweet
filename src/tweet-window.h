/* tweet-window.h: Main application window
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
#ifndef __TWEET_WINDOW_H__
#define __TWEET_WINDOW_H__

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

#define TWEET_TYPE_WINDOW               (tweet_window_get_type ())
#define TWEET_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_WINDOW, TweetWindow))
#define TWEET_IS_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_WINDOW))
#define TWEET_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_WINDOW, TweetWindowClass))
#define TWEET_IS_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_WINDOW))
#define TWEET_WINDOW_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_WINDOW, TweetWindowClass))

typedef struct _TweetWindow             TweetWindow;
typedef struct _TweetWindowPrivate      TweetWindowPrivate;
typedef struct _TweetWindowClass        TweetWindowClass;

struct _TweetWindow
{
  GtkWindow parent_instance;

  TweetWindowPrivate *priv;
};

struct _TweetWindowClass
{
  GtkWindowClass parent_class;
};

GType      tweet_window_get_type (void) G_GNUC_CONST;
GtkWidget *tweet_window_new      (void);

G_END_DECLS

#endif /* __TWEET_WINDOW_H__ */
