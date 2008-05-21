/* tweet-animation.h: Simple animation API
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
#ifndef __TWEET_ANIMATION_H__
#define __TWEET_ANIMATION_H__

#include <clutter/clutter-actor.h>

#include "tweet-interval.h"

G_BEGIN_DECLS

#define TWEET_TYPE_ANIMATION             (tweet_animation_get_type ())
#define TWEET_ANIMATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_ANIMATION, TweetAnimation))
#define TWEET_IS_ANIMATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_ANIMATION))
#define TWEET_ANIMATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_ANIMATION, TweetAnimationClass))
#define TWEET_IS_ANIMATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_ANIMATION))
#define TWEET_ANIMATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_ANIMATION, TweetAnimationClass))

typedef struct _TweetAnimation           TweetAnimation;
typedef struct _TweetAnimationPrivate    TweetAnimationPrivate;
typedef struct _TweetAnimationClass      TweetAnimationClass;

typedef enum {
  TWEET_LINEAR,
  TWEET_EASE_IN,
  TWEET_EASE_OUT
} TweetAnimationMode;

struct _TweetAnimation
{
  GInitiallyUnowned parent_instance;

  TweetAnimationPrivate *priv;
};

struct _TweetAnimationClass
{
  GInitiallyUnownedClass parent_class;

  void (* completed) (TweetAnimation *animation);
};

GType              tweet_animation_get_type        (void) G_GNUC_CONST;

TweetAnimation *   tweet_animation_new             (void);
void               tweet_animation_set_actor       (TweetAnimation     *animation,
                                                    ClutterActor       *actor);
ClutterActor *     tweet_animation_get_actor       (TweetAnimation     *animation);
void               tweet_animation_set_mode        (TweetAnimation     *animation,
                                                    TweetAnimationMode  mode);
TweetAnimationMode tweet_animation_get_mode        (TweetAnimation     *animation);
void               tweet_animation_set_duration    (TweetAnimation     *animation,
                                                   gint                 msecs);
guint              tweet_animation_get_duration    (TweetAnimation     *animation);
void               tweet_animation_set_loop        (TweetAnimation     *animation,
                                                   gboolean             loop);
gboolean           tweet_animation_get_loop        (TweetAnimation     *animation);
void               tweet_animation_bind_property   (TweetAnimation     *animation,
                                                    const gchar        *property_name,
                                                    TweetInterval      *interval);
gboolean           tweet_animation_has_property    (TweetAnimation     *animation,
                                                    const gchar        *property_name);
void               tweet_animation_unbind_property (TweetAnimation     *animation,
                                                    const gchar        *property_name);

void               tweet_animation_start           (TweetAnimation     *animation);
void               tweet_animation_stop            (TweetAnimation     *animation);

/* wrapper */
TweetAnimation *   tweet_actor_animate             (ClutterActor       *actor,
                                                    TweetAnimationMode  mode,
                                                    guint               duration,
                                                    const gchar        *first_property_name,
                                                    ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __TWEET_ANIMATION_H__ */
