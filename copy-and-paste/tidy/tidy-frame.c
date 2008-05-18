/* tidy-frame.c: Simple container with a background
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Emmanuele Bassi <ebassi@openedhand.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>

#include <clutter/cogl.h>

#include <clutter/clutter-container.h>
#include <clutter/clutter-texture.h>

#include "tidy-frame.h"
#include "tidy-private.h"
#include "tidy-stylable.h"

#define TIDY_FRAME_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TIDY_TYPE_FRAME, TidyFramePrivate))

enum
{
  PROP_0,

  PROP_CHILD,
  PROP_TEXTURE
};

struct _TidyFramePrivate
{
  ClutterActor *child;
  ClutterActor *texture;

  ClutterActorBox allocation;
};

static ClutterColor default_bg_color = { 0xcc, 0xcc, 0xcc, 0xff };

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (TidyFrame, tidy_frame, TIDY_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init));

static inline void
align_child (TidyFrame *frame)
{
  TidyFramePrivate *priv = frame->priv;
  TidyPadding padding = { 0, };
  ClutterFixed x_align, y_align;
  ClutterUnit width, height;
  ClutterUnit child_width, child_height;
  ClutterActorBox allocation = { 0, };
  ClutterActorBox child_req = { 0, };
  ClutterActorBox child_box = { 0, };

  tidy_actor_get_padding (TIDY_ACTOR (frame), &padding);
  tidy_actor_get_alignmentx (TIDY_ACTOR (frame), &x_align, &y_align);

  clutter_actor_query_coords (CLUTTER_ACTOR (frame), &allocation);

  width = allocation.x2 - allocation.x1
          - padding.left
          - padding.right;
  height = allocation.y2 - allocation.y1
           - padding.top
           - padding.bottom;

  if (width < 0)
    width = 0;

  if (height < 0)
    height = 0;

  clutter_actor_query_coords (priv->child, &child_req);

  child_width = child_req.x2 - child_req.x1;
  child_height = child_req.y2 - child_req.y1;

  child_box.x1 = CLUTTER_FIXED_MUL ((width - child_width), x_align)
                 + padding.left;
  child_box.y1 = CLUTTER_FIXED_MUL ((height - child_height), y_align)
                 + padding.top;

  child_box.x2 = child_box.x1 + child_width + 1;
  child_box.y2 = child_box.y1 + child_height + 1;

  clutter_actor_request_coords (priv->child, &child_box);
}

static void
tidy_frame_paint (ClutterActor *actor)
{
  TidyFrame *frame = TIDY_FRAME (actor);
  TidyFramePrivate *priv = frame->priv;

  cogl_push_matrix ();

  if (priv->texture)
    clutter_actor_paint (priv->texture);
  else
    {
      ClutterActorBox allocation = { 0, };
      ClutterColor *bg_color;
      guint w, h;

      clutter_actor_query_coords (actor, &allocation);

      w = CLUTTER_UNITS_TO_DEVICE (allocation.x2 - allocation.x1);
      h = CLUTTER_UNITS_TO_DEVICE (allocation.y2 - allocation.y1);

      tidy_stylable_get (TIDY_STYLABLE (frame), "bg-color", &bg_color, NULL);
      if (!bg_color)
        bg_color = &default_bg_color;

      bg_color->alpha = ((bg_color->alpha *
                          (gint)clutter_actor_get_opacity (actor)) / 255);

      cogl_enable (CGL_ENABLE_BLEND);
      cogl_color (bg_color);
      cogl_rectangle (0, 0, w, h);
      
      if (bg_color != &default_bg_color)
        clutter_color_free (bg_color);
    }

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);

  cogl_pop_matrix ();
}

static void
tidy_frame_pick (ClutterActor       *actor,
                 const ClutterColor *pick_color)
{
  /* chain up, so we get a box with our coordinates */
  CLUTTER_ACTOR_CLASS (tidy_frame_parent_class)->pick (actor, pick_color);

  /* paint our child, if any */
  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    {
      TidyFramePrivate *priv = TIDY_FRAME (actor)->priv;

      if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
        clutter_actor_paint (priv->child);
    }
}

static void
tidy_frame_request_coords (ClutterActor    *actor,
                           ClutterActorBox *box)
{
  TidyFramePrivate *priv = TIDY_FRAME (actor)->priv;

  priv->allocation = *box;

  if (priv->texture)
    {
      ClutterActorBox texture_box = { 0, };

      texture_box.x1 = texture_box.y1 = 0;
      texture_box.x2 = priv->allocation.x2 - priv->allocation.x1;
      texture_box.y2 = priv->allocation.y2 - priv->allocation.y1;

      clutter_actor_request_coords (priv->texture, &texture_box);
    }

  if (priv->child)
    align_child (TIDY_FRAME (actor));

  CLUTTER_ACTOR_CLASS (tidy_frame_parent_class)->request_coords (actor, box);
}

static void
tidy_frame_query_coords (ClutterActor    *actor,
                         ClutterActorBox *box)
{
  TidyFramePrivate *priv = TIDY_FRAME (actor)->priv;

  if (priv->allocation.x2 - priv->allocation.x1 <= 0 ||
      priv->allocation.y2 - priv->allocation.y1 <= 0)
    {
      TidyPadding padding = { 0, };

      tidy_actor_get_padding (TIDY_ACTOR (actor), &padding);

      box->x2 = box->x1 + padding.left + padding.right;
      box->y2 = box->y1 + padding.top + padding.bottom;

      if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
        {
          ClutterActorBox child_box = { 0, };

          clutter_actor_query_coords (priv->child, &child_box);

          box->x2 += (child_box.x2 - child_box.x1);
          box->y2 += (child_box.y2 - child_box.y1);
        }
    }
  else
    *box = priv->allocation;
}

static void
tidy_frame_dispose (GObject *gobject)
{
  TidyFramePrivate *priv = TIDY_FRAME (gobject)->priv;

  if (priv->child)
    {
      clutter_actor_unparent (priv->child);
      priv->child = NULL;
    }

  if (priv->texture)
    {
      clutter_actor_unparent (priv->texture);
      priv->texture = NULL;
    }

  G_OBJECT_CLASS (tidy_frame_parent_class)->dispose (gobject);
}

static void
tidy_frame_set_property (GObject      *gobject,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_CHILD:
      clutter_container_add_actor (CLUTTER_CONTAINER (gobject),
                                   g_value_get_object (value));
      break;

    case PROP_TEXTURE:
      tidy_frame_set_texture (TIDY_FRAME (gobject),
                              g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_frame_get_property (GObject    *gobject,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  TidyFramePrivate *priv = TIDY_FRAME (gobject)->priv;

  switch (prop_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, priv->child);
      break;

    case PROP_TEXTURE:
      g_value_set_object (value, priv->texture);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_frame_class_init (TidyFrameClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyFramePrivate));

  gobject_class->set_property = tidy_frame_set_property;
  gobject_class->get_property = tidy_frame_get_property;
  gobject_class->dispose = tidy_frame_dispose;

  actor_class->pick = tidy_frame_pick;
  actor_class->paint = tidy_frame_paint;
  actor_class->request_coords = tidy_frame_request_coords;
  actor_class->query_coords = tidy_frame_query_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_CHILD,
                                   g_param_spec_object ("child",
                                                        "Child",
                                                        "The child of the frame",
                                                        CLUTTER_TYPE_ACTOR,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_TEXTURE,
                                   g_param_spec_object ("texture",
                                                        "Texture",
                                                        "The background texture of the frame",
                                                        CLUTTER_TYPE_ACTOR,
                                                        TIDY_PARAM_READWRITE));
}

static void
tidy_frame_init (TidyFrame *frame)
{
  frame->priv = TIDY_FRAME_GET_PRIVATE (frame);
}

static void
tidy_frame_add_actor (ClutterContainer *container,
                      ClutterActor     *actor)
{
  TidyFramePrivate *priv = TIDY_FRAME (container)->priv;

  if (priv->child)
    clutter_actor_unparent (priv->child);

  clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));
  priv->child = actor;

  align_child (TIDY_FRAME (container));

  g_signal_emit_by_name (container, "actor-added", actor);

  g_object_notify (G_OBJECT (container), "child");
}

static void
tidy_frame_remove_actor (ClutterContainer *container,
                         ClutterActor     *actor)
{
  TidyFramePrivate *priv = TIDY_FRAME (container)->priv;

  if (priv->child == actor)
    {
      g_object_ref (priv->child);

      clutter_actor_unparent (priv->child);

      g_signal_emit_by_name (container, "actor-removed", priv->child);

      g_object_unref (priv->child);
      priv->child = NULL;

      if (CLUTTER_ACTOR_IS_VISIBLE (container))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
    }
}

static void
tidy_frame_foreach (ClutterContainer *container,
                    ClutterCallback   callback,
                    gpointer          callback_data)
{
  TidyFramePrivate *priv = TIDY_FRAME (container)->priv;

  if (priv->texture)
    callback (priv->texture, callback_data);

  if (priv->child)
    callback (priv->child, callback_data);
}

static void
tidy_frame_lower (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* single child */
}

static void
tidy_frame_raise (ClutterContainer *container,
                  ClutterActor     *actor,
                  ClutterActor     *sibling)
{
  /* single child */
}

static void
tidy_frame_sort_depth_order (ClutterContainer *container)
{
  /* single child */
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = tidy_frame_add_actor;
  iface->remove = tidy_frame_remove_actor;
  iface->foreach = tidy_frame_foreach;
  iface->lower = tidy_frame_lower;
  iface->raise = tidy_frame_raise;
  iface->sort_depth_order = tidy_frame_sort_depth_order;
}

ClutterActor *
tidy_frame_new (void)
{
  return g_object_new (TIDY_TYPE_FRAME, NULL);
}

ClutterActor *
tidy_frame_get_child (TidyFrame *frame)
{
  g_return_val_if_fail (TIDY_IS_FRAME (frame), NULL);

  return frame->priv->child;
}

void
tidy_frame_set_texture (TidyFrame    *frame,
                        ClutterActor *texture)
{
  TidyFramePrivate *priv;

  g_return_if_fail (TIDY_IS_FRAME (frame));
  g_return_if_fail (CLUTTER_IS_ACTOR (texture));

  priv = frame->priv;

  if (priv->texture == texture)
    return;

  if (priv->texture)
    {
      clutter_actor_unparent (priv->texture);
      priv->texture = NULL;
    }

  if (texture)
    {
      ClutterActor *parent = clutter_actor_get_parent (texture);
      ClutterActorBox box;

      if (G_UNLIKELY (parent))
        {
          g_warning ("Unable to set the background texture of type `%s' for "
                     "the frame of type `%s': the texture actor is already "
                     "a child of a container of type `%s'",
                     g_type_name (G_OBJECT_TYPE (texture)),
                     g_type_name (G_OBJECT_TYPE (frame)),
                     g_type_name (G_OBJECT_TYPE (parent)));
          return;
        }

      /* Set texture size */
      box.x1 = box.y1 = 0;
      clutter_actor_get_sizeu (CLUTTER_ACTOR (frame), &box.x2, &box.y2);
      clutter_actor_request_coords (texture, &box);

      clutter_actor_set_parent (texture, CLUTTER_ACTOR (frame));
      priv->texture = texture;
    }

  g_object_notify (G_OBJECT (frame), "texture");
}

ClutterActor *
tidy_frame_get_texture (TidyFrame *frame)
{
  g_return_val_if_fail (TIDY_IS_FRAME (frame), NULL);

  return frame->priv->texture;
}

