/* tweet-overlay.c: Overlay container
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

#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>

#include <gtk/gtk.h>

#include "tweet-overlay.h"
#include "tweet-utils.h"

#define BG_ROUND_RADIUS 24

#define TWEET_OVERLAY_GET_PRIVATE(obj)  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_OVERLAY, TweetOverlayPrivate))

struct _TweetOverlayPrivate
{
  ClutterActor *base;

  ClutterColor base_color;
};

enum
{
  PROP_0,

  PROP_BASE_COLOR
};

G_DEFINE_TYPE (TweetOverlay, tweet_overlay, CLUTTER_TYPE_GROUP);

static inline void
draw_background (TweetOverlay *overlay)
{
  TweetOverlayPrivate *priv = overlay->priv;
  cairo_t *cr;
  gint width, height;

  cr = clutter_cairo_create (CLUTTER_CAIRO (priv->base));
  g_assert (cr != NULL);

  width = height = 0;
  g_object_get (G_OBJECT (priv->base),
                "surface-width", &width,
                "surface-height", &height,
                NULL);

  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_move_to (cr, BG_ROUND_RADIUS, 0);
  cairo_line_to (cr, width - BG_ROUND_RADIUS, 0);
  cairo_curve_to (cr, width, 0, width, 0, width, BG_ROUND_RADIUS);
  cairo_line_to (cr, width, height - BG_ROUND_RADIUS);
  cairo_curve_to (cr, width, height, width, height, width - BG_ROUND_RADIUS, height);
  cairo_line_to (cr, BG_ROUND_RADIUS, height);
  cairo_curve_to (cr, 0, height, 0, height, 0, height - BG_ROUND_RADIUS);
  cairo_line_to (cr, 0, BG_ROUND_RADIUS);
  cairo_curve_to (cr, 0, 0, 0, 0, BG_ROUND_RADIUS, 0);

  cairo_close_path (cr);

  clutter_cairo_set_source_color (cr, &priv->base_color);
  cairo_fill_preserve (cr);

  cairo_destroy (cr);
}

static void
tweet_overlay_request_coords (ClutterActor    *actor,
                              ClutterActorBox *box)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  guint width, height;

  CLUTTER_ACTOR_CLASS (tweet_overlay_parent_class)->request_coords (actor, box);

  width = CLUTTER_UNITS_TO_DEVICE (box->x2 - box->x1);
  height = CLUTTER_UNITS_TO_DEVICE (box->y2 - box->y1);

  clutter_cairo_surface_resize (CLUTTER_CAIRO (priv->base), width, height);
  draw_background (TWEET_OVERLAY (actor));
}

static void
tweet_overlay_query_coords (ClutterActor    *actor,
                            ClutterActorBox *box)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  gint width, height;

  /* we are as big as the base actor */
  width = height = 0;
  g_object_get (G_OBJECT (priv->base),
                "surface-width", &width,
                "surface-height", &height,
                NULL);

  box->x2 = box->x1 + CLUTTER_UNITS_FROM_DEVICE (width);
  box->y2 = box->y1 + CLUTTER_UNITS_FROM_DEVICE (height);
}

static void
tweet_overlay_dispose (GObject *gobject)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (gobject)->priv;

  if (priv->base)
    {
      clutter_actor_destroy (priv->base);
      priv->base = NULL;
    }

  G_OBJECT_CLASS (tweet_overlay_parent_class)->dispose (gobject);
}

static void
tweet_overlay_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  TweetOverlay *overlay = TWEET_OVERLAY (gobject);

  switch (prop_id)
    {
    case PROP_BASE_COLOR:
      tweet_overlay_set_color (overlay, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_overlay_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (gobject)->priv;

  switch (prop_id)
    {
    case PROP_BASE_COLOR:
      g_value_set_boxed (value, &priv->base_color);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_overlay_constructed (GObject *gobject)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (gobject)->priv;

  priv->base = clutter_cairo_new (128, 128);
  draw_background (TWEET_OVERLAY (gobject));
  clutter_actor_set_size (priv->base, 128, 128);
  clutter_actor_set_position (priv->base, 0, 0);
  clutter_container_add_actor (CLUTTER_CONTAINER (gobject), priv->base);
  clutter_actor_show (priv->base);
}

static void
tweet_overlay_class_init (TweetOverlayClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetOverlayPrivate));

  gobject_class->constructed = tweet_overlay_constructed;
  gobject_class->set_property = tweet_overlay_set_property;
  gobject_class->get_property = tweet_overlay_get_property;
  gobject_class->dispose = tweet_overlay_dispose;

  actor_class->request_coords = tweet_overlay_request_coords;
  actor_class->query_coords = tweet_overlay_query_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_BASE_COLOR,
                                   g_param_spec_boxed ("base-color",
                                                       "Base Color",
                                                       "Background color",
                                                       CLUTTER_TYPE_COLOR,
                                                       G_PARAM_READWRITE));
}

static void
tweet_overlay_init (TweetOverlay *overlay)
{
  TweetOverlayPrivate *priv;

  overlay->priv = priv = TWEET_OVERLAY_GET_PRIVATE (overlay);

  priv->base_color.red   = 0x44;
  priv->base_color.green = 0x44;
  priv->base_color.blue  = 0x44;
  priv->base_color.alpha = 0xff;
}

ClutterActor *
tweet_overlay_new (void)
{
  return g_object_new (TWEET_TYPE_OVERLAY, NULL);
}

void
tweet_overlay_set_color (TweetOverlay       *overlay,
                         const ClutterColor *color)
{
  TweetOverlayPrivate *priv;

  g_return_if_fail (TWEET_IS_OVERLAY (overlay));
  g_return_if_fail (color != NULL);

  priv = overlay->priv;

  priv->base_color = *color;

  g_object_notify (G_OBJECT (overlay), "bg-color");
}

void
tweet_overlay_get_color (TweetOverlay *overlay,
                         ClutterColor *color)
{
  TweetOverlayPrivate *priv;

  g_return_if_fail (TWEET_IS_OVERLAY (overlay));
  g_return_if_fail (color != NULL);

  priv = overlay->priv;

  *color = priv->base_color;
}
