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

  GList *children;
};

enum
{
  PROP_0,

  PROP_BASE_COLOR
};

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (TweetOverlay, tweet_overlay, TIDY_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init));

static inline void
draw_background (TweetOverlay *overlay)
{
  TweetOverlayPrivate *priv = overlay->priv;
  cairo_t *cr;
  gint width, height;

  g_assert (CLUTTER_IS_CAIRO (priv->base));

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

/* ClutterActor methods */
static void
tweet_overlay_request_coords (ClutterActor    *actor,
                              ClutterActorBox *box)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  guint width, height;

  CLUTTER_ACTOR_CLASS (tweet_overlay_parent_class)->request_coords (actor, box);

  if (G_LIKELY (priv->base))
    {
      width = CLUTTER_UNITS_TO_DEVICE (box->x2 - box->x1);
      height = CLUTTER_UNITS_TO_DEVICE (box->y2 - box->y1);

      clutter_actor_set_size (priv->base, width, height);
      clutter_cairo_surface_resize (CLUTTER_CAIRO (priv->base),
                                    width, height);

      draw_background (TWEET_OVERLAY (actor));
   }
}

static void
tweet_overlay_query_coords (ClutterActor    *actor,
                            ClutterActorBox *box)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  gint width, height;

  if (!priv->base)
    {
      box->x2 = box->x1;
      box->y2 = box->y1;
      return;
    }

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
tweet_overlay_paint (ClutterActor *actor)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  GList *l;

  if (G_LIKELY (priv->base))
    clutter_actor_paint (priv->base);

  for (l = priv->children; l != NULL; l = l->next)
    {
      ClutterActor *child = l->data;

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }
}

static void
tweet_overlay_pick (ClutterActor       *actor,
                    const ClutterColor *pick_color)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;
  GList *l;

  CLUTTER_ACTOR_CLASS (tweet_overlay_parent_class)->pick (actor, pick_color);

  for (l = priv->children; l != NULL; l = l->next)
    {
      ClutterActor *child = l->data;

      if (CLUTTER_ACTOR_IS_VISIBLE (child))
        clutter_actor_paint (child);
    }
}

static void
tweet_overlay_realize (ClutterActor *actor)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;

  if (G_LIKELY (priv->base))
    clutter_actor_realize (priv->base);

  g_list_foreach (priv->children,
                  (GFunc) clutter_actor_realize,
                  NULL);
}

static void
tweet_overlay_unrealize (ClutterActor *actor)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (actor)->priv;

  if (G_LIKELY (priv->base))
    clutter_actor_unrealize (priv->base);

  g_list_foreach (priv->children,
                  (GFunc) clutter_actor_unrealize,
                  NULL);
}

/* ClutterContainer methods */
static void
tweet_overlay_add (ClutterContainer *container,
                   ClutterActor     *actor)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;

  priv->children = g_list_append (priv->children, actor);
  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

  g_signal_emit_by_name (container, "actor-added", actor);

  clutter_container_sort_depth_order (container);
}

static void
tweet_overlay_remove (ClutterContainer *container,
                      ClutterActor     *actor)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;

  g_object_ref (actor);

  priv->children = g_list_remove (priv->children, actor);
  clutter_actor_unparent (actor);

  g_signal_emit_by_name (container, "actor-removed", actor);

  clutter_container_sort_depth_order (container);

  g_object_unref (actor);
}

static void
tweet_overlay_foreach (ClutterContainer *container,
                       ClutterCallback   callback,
                       gpointer          callback_data)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;
  GList *l;

  if (priv->base)
    (* callback) (priv->base, callback_data);

  for (l = priv->children; l != NULL; l = l->next)
    (* callback) (l->data, callback_data);
}

static void
tweet_overlay_raise (ClutterContainer *container,
                     ClutterActor     *actor,
                     ClutterActor     *sibling)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;

  priv->children = g_list_remove (priv->children, actor);

  /* Raise at the top */
  if (!sibling)
    {
      GList *last_item;

      last_item = g_list_last (priv->children);

      if (last_item)
        sibling = last_item->data;

      priv->children = g_list_append (priv->children, actor);
    }
  else
    {
      gint pos;

      pos = g_list_index (priv->children, sibling) + 1;

      priv->children = g_list_insert (priv->children, actor, pos);
    }

  if (sibling &&
      clutter_actor_get_depth (sibling) != clutter_actor_get_depth (actor))
    {
      clutter_actor_set_depth (actor, clutter_actor_get_depth (sibling));
    }
}

static void
tweet_overlay_lower (ClutterContainer *container,
                     ClutterActor     *actor,
                     ClutterActor     *sibling)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;

  priv->children = g_list_remove (priv->children, actor);

  /* Push to bottom */
  if (!sibling)
    {
      GList *last_item;

      last_item = g_list_first (priv->children);

      if (last_item)
        sibling = last_item->data;

      priv->children = g_list_prepend (priv->children, actor);
    }
  else
    {
      gint pos;

      pos = g_list_index (priv->children, sibling);

      priv->children = g_list_insert (priv->children, actor, pos);
    }

  if (sibling &&
      clutter_actor_get_depth (sibling) != clutter_actor_get_depth (actor))
    {
      clutter_actor_set_depth (actor, clutter_actor_get_depth (sibling));
    }
}

static gint
sort_z_order (gconstpointer a,
              gconstpointer b)
{
  gint depth_a, depth_b;

  depth_a = clutter_actor_get_depth ((ClutterActor *) a);
  depth_b = clutter_actor_get_depth ((ClutterActor *) b);

  return (depth_a - depth_b);
}

static void
tweet_overlay_sort_depth_order (ClutterContainer *container)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (container)->priv;

  priv->children = g_list_sort (priv->children, sort_z_order);

  if (CLUTTER_ACTOR_IS_VISIBLE (container))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

/* GObject methods */
static void
tweet_overlay_dispose (GObject *gobject)
{
  TweetOverlayPrivate *priv = TWEET_OVERLAY (gobject)->priv;

  if (priv->base)
    {
      clutter_actor_destroy (priv->base);
      priv->base = NULL;
    }

  g_list_foreach (priv->children, (GFunc) clutter_actor_destroy, NULL);
  g_list_free (priv->children);
  priv->children = NULL;

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
  clutter_actor_set_parent (priv->base, CLUTTER_ACTOR (gobject));

  draw_background (TWEET_OVERLAY (gobject));
  clutter_actor_set_size (priv->base, 128, 128);
  clutter_actor_set_position (priv->base, 0, 0);
  clutter_actor_show (priv->base);
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = tweet_overlay_add;
  iface->remove = tweet_overlay_remove;
  iface->foreach = tweet_overlay_foreach;
  iface->raise = tweet_overlay_raise;
  iface->lower = tweet_overlay_lower;
  iface->sort_depth_order = tweet_overlay_sort_depth_order;
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

  actor_class->realize = tweet_overlay_realize;
  actor_class->unrealize = tweet_overlay_unrealize;
  actor_class->paint = tweet_overlay_paint;
  actor_class->pick = tweet_overlay_pick;
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
  draw_background (overlay);

  if (CLUTTER_ACTOR_IS_VISIBLE (overlay))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (overlay));

  g_object_notify (G_OBJECT (overlay), "base-color");
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
