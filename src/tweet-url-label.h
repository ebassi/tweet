/* tweet-url-label.h: Subclass of ClutterLabel with clickable URLs
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
#ifndef __TWEET_URL_LABEL_H__
#define __TWEET_URL_LABEL_H__

#include <clutter/clutter-label.h>
#include <glib/gregex.h>
#include <gdk/gdkcursor.h>

G_BEGIN_DECLS

#define TWEET_TYPE_URL_LABEL                                            \
  (tweet_url_label_get_type())
#define TWEET_URL_LABEL(obj)                                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               TWEET_TYPE_URL_LABEL,                    \
                               TweetUrlLabel))
#define TWEET_URL_LABEL_CLASS(klass)                                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            TWEET_TYPE_URL_LABEL,                       \
                            TweetUrlLabelClass))
#define TWEET_IS_URL_LABEL(obj)                                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               TWEET_TYPE_URL_LABEL))
#define TWEET_IS_URL_LABEL_CLASS(klass)                                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            TWEET_TYPE_URL_LABEL))
#define TWEET_URL_LABEL_GET_CLASS(obj)                                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              TWEET_TYPE_URL_LABEL,                     \
                              TweetUrlLabelClass))

typedef struct _TweetUrlLabel      TweetUrlLabel;
typedef struct _TweetUrlLabelClass TweetUrlLabelClass;

struct _TweetUrlLabelClass
{
  ClutterLabelClass parent_class;
};

struct _TweetUrlLabel
{
  ClutterLabel parent;
  
  GRegex *url_regex;
  
  GArray *matches;
  gint selected_match;

  /* Cache a reference to the hand cursor */
  GdkCursor *hand_cursor;
};

GType tweet_url_label_get_type (void) G_GNUC_CONST;

ClutterActor *tweet_url_label_new (void);

G_END_DECLS

#endif /* __TWEET_URL_LABEL_H__ */
