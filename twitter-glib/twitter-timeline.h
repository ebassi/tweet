/* twitter-status.h: Collection of statuses
 *
 * This file is part of Twitter-GLib.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __TWITTER_TIMELINE_H__
#define __TWITTER_TIMELINE_H__

#include <glib-object.h>
#include <twitter-glib/twitter-status.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_TIMELINE            (twitter_timeline_get_type ())
#define TWITTER_TIMELINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_TIMELINE, TwitterTimeline))
#define TWITTER_IS_TIMELINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_TIMELINE))
#define TWITTER_TIMELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_TIMELINE, TwitterTimelineClass))
#define TWITTER_IS_TIMELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_TIMELINE))
#define TWITTER_TIMELINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_TIMELINE, TwitterTimelineClass))

typedef struct _TwitterTimeline         TwitterTimeline;
typedef struct _TwitterTimelinePrivate  TwitterTimelinePrivate;
typedef struct _TwitterTimelineClass    TwitterTimelineClass;

struct _TwitterTimeline
{
  GObject parent_instance;

  TwitterTimelinePrivate *priv;
};

struct _TwitterTimelineClass
{
  GObjectClass parent_class;
};

GType            twitter_timeline_get_type       (void) G_GNUC_CONST;

TwitterTimeline *twitter_timeline_new            (void);
TwitterTimeline *twitter_timeline_new_from_data  (const gchar     *buffer);

void             twitter_timeline_load_from_data (TwitterTimeline *timeline,
                                                  const gchar     *buffer);

guint            twitter_timeline_get_count      (TwitterTimeline *timeline);
TwitterStatus *  twitter_timeline_get_id         (TwitterTimeline *timeline,
                                                  guint            id);
TwitterStatus *  twitter_timeline_get_pos        (TwitterTimeline *timeline,
                                                  gint             index_);
GList *          twitter_timeline_get_all        (TwitterTimeline *timeline);

G_END_DECLS

#endif /* __TWITTER_TIMELINE_H__ */
