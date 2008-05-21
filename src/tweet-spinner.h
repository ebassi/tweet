/* tweet-spinner.h: Spinning actor for long jobs
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
#ifndef __TWEET_SPINNER_H__
#define __TWEET_SPINNER_H__

#include <clutter/clutter-actor.h>
#include <clutter-cairo/clutter-cairo.h>

G_BEGIN_DECLS

#define TWEET_TYPE_SPINNER              (tweet_spinner_get_type ())
#define TWEET_SPINNER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_SPINNER, TweetSpinner))
#define TWEET_IS_SPINNER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_SPINNER))
#define TWEET_SPINNER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_SPINNER, TweetSpinnerClass))
#define TWEET_IS_SPINNER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_SPINNER))
#define TWEET_SPINNER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_SPINNER, TweetSpinnerClass))

typedef struct _TweetSpinner            TweetSpinner;
typedef struct _TweetSpinnerPrivate     TweetSpinnerPrivate;
typedef struct _TweetSpinnerClass       TweetSpinnerClass;

struct _TweetSpinner
{
  ClutterGroup parent_instance;

  TweetSpinnerPrivate *priv;
};

struct _TweetSpinnerClass
{
  ClutterGroupClass parent_class;
};

GType         tweet_spinner_get_type  (void) G_GNUC_CONST;
ClutterActor *tweet_spinner_new       (void);
void          tweet_spinner_set_image (TweetSpinner       *spinner,
                                       ClutterActor       *image);
ClutterActor *tweet_spinner_get_image (TweetSpinner       *spinner);
void          tweet_spinner_set_color (TweetSpinner       *spinner,
                                       const ClutterColor *color);
void          tweet_spinner_get_color (TweetSpinner       *spinner,
                                       ClutterColor       *color);
void          tweet_spinner_start     (TweetSpinner       *spinner);
void          tweet_spinner_stop      (TweetSpinner       *spinner);

G_END_DECLS

#endif /* __TWEET_SPINNER_H__ */
