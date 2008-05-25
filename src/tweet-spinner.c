/* tweet-spinner.c: Spinning actor for long jobs
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/cogl.h>
#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>

#include <gtk/gtk.h>

#include "tweet-spinner.h"
#include "tweet-utils.h"

#define BG_ROUND_RADIUS 24

#define TWEET_SPINNER_GET_PRIVATE(obj)  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_SPINNER, TweetSpinnerPrivate))

struct _TweetSpinnerPrivate
{
  ClutterActor *image;

  ClutterTimeline *timeline;
  ClutterBehaviour *rotate_b;
};

enum
{
  PROP_0,

  PROP_IMAGE
};

G_DEFINE_TYPE (TweetSpinner, tweet_spinner, TWEET_TYPE_OVERLAY);

static void
tweet_spinner_request_coords (ClutterActor *actor,
                              ClutterActorBox *box)
{
  TweetSpinnerPrivate *priv = TWEET_SPINNER (actor)->priv;
  ClutterUnit width, height;

  CLUTTER_ACTOR_CLASS (tweet_spinner_parent_class)->request_coords (actor, box);

  width = box->x2 - box->x1;
  height = box->y2 - box->y1;

  if (priv->image)
    clutter_actor_set_positionu (priv->image, width / 2, height / 2);
}

static void
tweet_spinner_dispose (GObject *gobject)
{
  TweetSpinnerPrivate *priv = TWEET_SPINNER (gobject)->priv;

  if (priv->timeline)
    {
      clutter_timeline_stop (priv->timeline);
      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }

  if (priv->rotate_b)
    {
      g_object_unref (priv->rotate_b);
      priv->rotate_b = NULL;
    }

  if (priv->image)
    {
      clutter_actor_destroy (priv->image);
      priv->image = NULL;
    }

  G_OBJECT_CLASS (tweet_spinner_parent_class)->dispose (gobject);
}

static void
tweet_spinner_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  TweetSpinner *spinner = TWEET_SPINNER (gobject);

  switch (prop_id)
    {
    case PROP_IMAGE:
      tweet_spinner_set_image (spinner, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_spinner_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  TweetSpinnerPrivate *priv = TWEET_SPINNER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_IMAGE:
      g_value_set_object (value, priv->image);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_spinner_class_init (TweetSpinnerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetSpinnerPrivate));

  gobject_class->set_property = tweet_spinner_set_property;
  gobject_class->get_property = tweet_spinner_get_property;
  gobject_class->dispose = tweet_spinner_dispose;

  actor_class->request_coords = tweet_spinner_request_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_IMAGE,
                                   g_param_spec_object ("image",
                                                        "Image",
                                                        "Spinning image",
                                                        CLUTTER_TYPE_ACTOR,
                                                        G_PARAM_READWRITE));
}

static void
tweet_spinner_init (TweetSpinner *spinner)
{
  TweetSpinnerPrivate *priv;

  spinner->priv = priv = TWEET_SPINNER_GET_PRIVATE (spinner);

  priv->timeline = clutter_timeline_new_for_duration (1000);
  clutter_timeline_set_loop (priv->timeline, TRUE);

  priv->rotate_b = clutter_behaviour_rotate_new (clutter_alpha_new_full (priv->timeline,
                                                                         CLUTTER_ALPHA_RAMP_INC,
                                                                         NULL,
                                                                         NULL),
                                                 CLUTTER_Z_AXIS,
                                                 CLUTTER_ROTATE_CW,
                                                 0.0, 360.0);
}

ClutterActor *
tweet_spinner_new (void)
{
  return g_object_new (TWEET_TYPE_SPINNER, NULL);
}

void
tweet_spinner_set_image (TweetSpinner *spinner,
                         ClutterActor *image)
{
  TweetSpinnerPrivate *priv;
  guint img_width, img_height;

  g_return_if_fail (TWEET_IS_SPINNER (spinner));
  g_return_if_fail (image == NULL || CLUTTER_IS_ACTOR (image));

  priv = spinner->priv;

  if (priv->image)
    {
      clutter_behaviour_remove (priv->rotate_b, priv->image);
      clutter_actor_destroy (priv->image);
      priv->image = NULL;
    }

  if (!image)
    {
      ClutterColor white = { 255, 255, 255, 255 };

      priv->image = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (priv->image), &white);
      clutter_actor_set_size (priv->image, 48, 48);
    }
  else
    priv->image = image;

  img_width = clutter_actor_get_widthu (priv->image);
  img_height = clutter_actor_get_heightu (priv->image);

  clutter_container_add_actor (CLUTTER_CONTAINER (spinner), priv->image);
  clutter_actor_set_anchor_pointu (priv->image,
                                   img_width / 2,
                                   img_height / 2);
  clutter_actor_show (priv->image);

  clutter_behaviour_apply (priv->rotate_b, priv->image);

  g_object_notify (G_OBJECT (spinner), "image");
}

ClutterActor *
tweet_spinner_get_image (TweetSpinner *spinner)
{
  g_return_val_if_fail (TWEET_IS_SPINNER (spinner), NULL);

  return spinner->priv->image;
}

void
tweet_spinner_start (TweetSpinner *spinner)
{
  g_return_if_fail (TWEET_IS_SPINNER (spinner));

  clutter_timeline_start (spinner->priv->timeline);
}

void
tweet_spinner_stop (TweetSpinner *spinner)
{
  g_return_if_fail (TWEET_IS_SPINNER (spinner));

  clutter_timeline_stop (spinner->priv->timeline);
}

