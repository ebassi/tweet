/* tweet-overlay.h: Overlay container
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
#ifndef __TWEET_OVERLAY_H__
#define __TWEET_OVERLAY_H__

#include <clutter/clutter-actor.h>

G_BEGIN_DECLS

#define TWEET_TYPE_OVERLAY              (tweet_overlay_get_type ())
#define TWEET_OVERLAY(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_OVERLAY, TweetOverlay))
#define TWEET_IS_OVERLAY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_OVERLAY))
#define TWEET_OVERLAY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_OVERLAY, TweetOverlayClass))
#define TWEET_IS_OVERLAY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_OVERLAY))
#define TWEET_OVERLAY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_OVERLAY, TweetOverlayClass))

typedef struct _TweetOverlay            TweetOverlay;
typedef struct _TweetOverlayPrivate     TweetOverlayPrivate;
typedef struct _TweetOverlayClass       TweetOverlayClass;

struct _TweetOverlay
{
  ClutterGroup parent_instance;

  TweetOverlayPrivate *priv;
};

struct _TweetOverlayClass
{
  ClutterGroupClass parent_class;
};

GType         tweet_overlay_get_type  (void) G_GNUC_CONST;
ClutterActor *tweet_overlay_new       (void);
void          tweet_overlay_set_color (TweetOverlay       *overlay,
                                       const ClutterColor *color);
void          tweet_overlay_get_color (TweetOverlay       *overlay,
                                       ClutterColor       *color);

G_END_DECLS

#endif /* __TWEET_OVERLAY_H__ */
