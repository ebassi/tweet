/* tweet-hot-actor.c: Interface to an actor with a dynamic cursor
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
#ifndef __TWEET_HOT_ACTOR_H__
#define __TWEET_HOT_ACTOR_H__

#include <glib-object.h>
#include <gdk/gdkdisplay.h>
#include <gdk/gdkcursor.h>

G_BEGIN_DECLS

/* TweetHotActor is an interface to an actor that can report a desired
   cursor for a given stage-relative coordinate */

#define TWEET_TYPE_HOT_ACTOR			\
  (tweet_hot_actor_get_type ())
#define TWEET_HOT_ACTOR(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_HOT_ACTOR, TweetHotActor))
#define TWEET_IS_HOT_ACTOR(obj)					\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_HOT_ACTOR))
#define TWEET_HOT_ACTOR_GET_IFACE(obj)				\
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TWEET_TYPE_HOT_ACTOR,	\
				  TweetHotActorIface))

typedef struct _TweetHotActorIface   TweetHotActorIface;
typedef struct _TweetHotActor        TweetHotActor;

struct _TweetHotActorIface
{
  /*< private >*/
  GTypeInterface g_iface;

  /*< public >*/
  GdkCursor * (* get_cursor) (TweetHotActor *actor,
			      GdkDisplay    *display,
			      int            x,
			      int            y);
};

GType         tweet_hot_actor_get_type         (void) G_GNUC_CONST;

GdkCursor *   tweet_hot_actor_get_cursor (TweetHotActor *actor,
					  GdkDisplay    *display,
					  int            x,
					  int            y);

G_END_DECLS

#endif /* __TWEET_HOT_ACTOR_H__ */
