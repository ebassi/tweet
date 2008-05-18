/* tidy-cell-renderer.c: Base class for cell renderers
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

#include <stdlib.h>
#include <glib.h>

#include "tidy-cell-renderer.h"
#include "tidy-debug.h"
#include "tidy-private.h"

struct _TidyCellRendererPrivate
{
  ClutterFixed x_align;
  ClutterFixed y_align;
};

enum
{
  PROP_0,

  PROP_X_ALIGN,
  PROP_Y_ALIGN
};

G_DEFINE_ABSTRACT_TYPE (TidyCellRenderer,
                        tidy_cell_renderer,
                        G_TYPE_INITIALLY_UNOWNED);

static void
tidy_cell_renderer_set_property (GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  TidyCellRenderer *renderer = TIDY_CELL_RENDERER (gobject);
  ClutterFixed align = 0;

  switch (prop_id)
    {
    case PROP_X_ALIGN:
      align = CLUTTER_FLOAT_TO_FIXED (g_value_get_double (value));
      tidy_cell_renderer_set_alignmentx (renderer,
                                         align,
                                         renderer->priv->y_align);
      break;

    case PROP_Y_ALIGN:
      align = CLUTTER_FLOAT_TO_FIXED (g_value_get_double (value));
      tidy_cell_renderer_set_alignmentx (renderer,
                                         renderer->priv->x_align,
                                         align);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_cell_renderer_get_property (GObject    *gobject,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  TidyCellRendererPrivate *priv = TIDY_CELL_RENDERER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_X_ALIGN:
      g_value_set_double (value, CLUTTER_FIXED_TO_FLOAT (priv->x_align));
      break;

    case PROP_Y_ALIGN:
      g_value_set_double (value, CLUTTER_FIXED_TO_FLOAT (priv->y_align));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static ClutterActor *
tidy_cell_renderer_real_get_cell_actor (TidyCellRenderer *renderer,
                                        TidyActor        *list_view,
                                        const GValue     *value,
                                        TidyCellState     state,
                                        ClutterGeometry  *size,
                                        gint              row,
                                        gint              column)
{
  g_warning ("The cell renderer of type `%s' does not implement "
             "the TidyCellRenderer::get_cell_actor() virtual function.",
             g_type_name (G_OBJECT_TYPE (renderer)));
  return NULL;
}

static void
tidy_cell_renderer_class_init (TidyCellRendererClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyCellRendererPrivate));

  gobject_class->set_property = tidy_cell_renderer_set_property;
  gobject_class->get_property = tidy_cell_renderer_get_property;

  g_object_class_install_property (gobject_class,
                                   PROP_X_ALIGN,
                                   g_param_spec_double ("x-align",
                                                        "X Align",
                                                        "The alignment on the X axis, between 0.0 (left) and 1.0 (right)",
                                                        0.0, 1.0, 0.5,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_Y_ALIGN,
                                   g_param_spec_double ("y-align",
                                                        "Y Align",
                                                        "The alignment on the Y axis, between 0.0 (top) and 1.0 (bottom)",
                                                        0.0, 1.0, 0.5,
                                                        TIDY_PARAM_READWRITE));

  klass->get_cell_actor = tidy_cell_renderer_real_get_cell_actor;
}

static void
tidy_cell_renderer_init (TidyCellRenderer *renderer)
{
  renderer->priv = G_TYPE_INSTANCE_GET_PRIVATE (renderer,
                                                TIDY_TYPE_CELL_RENDERER,
                                                TidyCellRendererPrivate);

  renderer->priv->x_align = CLUTTER_FLOAT_TO_FIXED (0.5);
  renderer->priv->y_align = CLUTTER_FLOAT_TO_FIXED (0.5);
}

ClutterActor *
tidy_cell_renderer_get_cell_actor (TidyCellRenderer *renderer,
                                   TidyActor        *list_view,
                                   const GValue     *value,
                                   TidyCellState     state,
                                   ClutterGeometry  *size,
                                   gint              row,
                                   gint              column)
{
  TidyCellRendererClass *klass;

  g_return_val_if_fail (TIDY_IS_CELL_RENDERER (renderer), NULL);
  g_return_val_if_fail (TIDY_IS_ACTOR (list_view), NULL);
  g_return_val_if_fail (value != NULL, NULL);

  klass = TIDY_CELL_RENDERER_GET_CLASS (renderer);
  return klass->get_cell_actor (renderer, list_view,
                                value,
                                state,
                                size,
                                row, column);
}

void
tidy_cell_renderer_get_alignment (TidyCellRenderer *renderer,
                                  gdouble          *x_align,
                                  gdouble          *y_align)
{
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  if (x_align)
    *x_align = CLUTTER_FIXED_TO_FLOAT (renderer->priv->x_align);

  if (y_align)
    *y_align = CLUTTER_FIXED_TO_FLOAT (renderer->priv->y_align);
}

void
tidy_cell_renderer_get_alignmentx (TidyCellRenderer *renderer,
                                   ClutterFixed     *x_align,
                                   ClutterFixed     *y_align)
{
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  if (x_align)
    *x_align = renderer->priv->x_align;

  if (y_align)
    *y_align = renderer->priv->y_align;
}

void
tidy_cell_renderer_set_alignment (TidyCellRenderer *renderer,
                                  gdouble           x_align,
                                  gdouble           y_align)
{
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  x_align = CLAMP (x_align, 0.0, 1.0);
  y_align = CLAMP (y_align, 0.0, 1.0);

  tidy_cell_renderer_set_alignmentx (renderer,
                                     CLUTTER_FLOAT_TO_FIXED (x_align),
                                     CLUTTER_FLOAT_TO_FIXED (y_align));
}

void
tidy_cell_renderer_set_alignmentx (TidyCellRenderer *renderer,
                                   ClutterFixed      x_align,
                                   ClutterFixed      y_align)
{
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  x_align = CLAMP (x_align, 0, CFX_ONE);
  y_align = CLAMP (y_align, 0, CFX_ONE);

  g_object_freeze_notify (G_OBJECT (renderer));

  if (x_align != renderer->priv->x_align)
    {
      renderer->priv->x_align = x_align;
      g_object_notify (G_OBJECT (renderer), "x-align");
    }

  if (y_align != renderer->priv->y_align)
    {
      renderer->priv->y_align = y_align;
      g_object_notify (G_OBJECT (renderer), "y-align");
    }

  g_object_thaw_notify (G_OBJECT (renderer));
}
