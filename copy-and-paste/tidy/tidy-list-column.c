/* tidy-list-column.c: Base class for list columns
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
#include <string.h>

#include "tidy-cell-renderer.h"
#include "tidy-list-column.h"
#include "tidy-list-view.h"
#include "tidy-style.h"

#include "tidy-debug.h"
#include "tidy-enum-types.h"
#include "tidy-private.h"

#define TIDY_LIST_COLUMN_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TIDY_TYPE_LIST_COLUMN, TidyListColumnPrivate))

struct _TidyListColumnPrivate
{
  TidyCellRenderer *header;
  TidyCellRenderer *cell;

  TidyListView *list_view;

  ClutterUnit max_width;
  ClutterUnit min_width;

  guint model_id;

  guint is_visible : 1;
};

enum
{
  PROP_0,

  PROP_LIST_VIEW,
  PROP_HEADER_RENDERER,
  PROP_CELL_RENDERER,
  PROP_MAX_WIDTH,
  PROP_MIN_WIDTH,
  PROP_WIDTH,
  PROP_MODEL_INDEX,
  PROP_TITLE,
  PROP_VISIBLE
};

G_DEFINE_ABSTRACT_TYPE (TidyListColumn,
                        tidy_list_column,
                        G_TYPE_INITIALLY_UNOWNED);

static void
tidy_list_column_dispose (GObject *gobject)
{
  TidyListColumnPrivate *priv = TIDY_LIST_COLUMN (gobject)->priv;

  if (priv->cell)
    {
      g_object_unref (priv->cell);
      priv->cell = NULL;
    }

  if (priv->header)
    {
      g_object_unref (priv->header);
      priv->header = NULL;
    }

  G_OBJECT_CLASS (tidy_list_column_parent_class)->dispose (gobject);
}

static void
tidy_list_column_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  TidyListColumn *column = TIDY_LIST_COLUMN (gobject);

  switch (prop_id)
    {
    case PROP_HEADER_RENDERER:
      tidy_list_column_set_header_renderer (column, g_value_get_object (value));
      break;
    case PROP_CELL_RENDERER:
      tidy_list_column_set_cell_renderer (column, g_value_get_object (value));
      break;
    case PROP_MAX_WIDTH:
      tidy_list_column_set_max_width (column, g_value_get_int (value));
      break;
    case PROP_MIN_WIDTH:
      tidy_list_column_set_min_width (column, g_value_get_int (value));
      break;
    case PROP_MODEL_INDEX:
      column->priv->model_id = g_value_get_uint (value);
      break;
    case PROP_LIST_VIEW:
      column->priv->list_view = g_value_get_object (value);
      break;
    case PROP_VISIBLE:
      column->priv->is_visible = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_list_column_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  TidyListColumnPrivate *priv = TIDY_LIST_COLUMN_GET_PRIVATE (gobject);

  switch (prop_id)
    {
    case PROP_HEADER_RENDERER:
      g_value_set_object (value, priv->header);
      break;
    case PROP_CELL_RENDERER:
      g_value_set_object (value, priv->cell);
      break;
    case PROP_MAX_WIDTH:
      g_value_set_int (value, CLUTTER_UNITS_FROM_DEVICE (priv->max_width));
      break;
    case PROP_MIN_WIDTH:
      g_value_set_int (value, CLUTTER_UNITS_FROM_DEVICE (priv->min_width));
      break;
    case PROP_WIDTH:
      g_value_set_uint (value, tidy_list_column_get_width (TIDY_LIST_COLUMN (gobject)));
      break;
    case PROP_MODEL_INDEX:
      g_value_set_uint (value, priv->model_id);
      break;
    case PROP_TITLE:
      g_value_set_string (value, tidy_list_column_get_title (TIDY_LIST_COLUMN (gobject)));
      break;
    case PROP_LIST_VIEW:
      g_value_set_object (value, priv->list_view);
      break;
    case PROP_VISIBLE:
      g_value_set_boolean (value, priv->is_visible);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_list_column_class_init (TidyListColumnClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyListColumnPrivate));

  gobject_class->set_property = tidy_list_column_set_property;
  gobject_class->get_property = tidy_list_column_get_property;
  gobject_class->dispose = tidy_list_column_dispose;

  g_object_class_install_property (gobject_class,
                                   PROP_LIST_VIEW,
                                   g_param_spec_object ("list-view",
                                                        "List View",
                                                        "The parent list view",
                                                        TIDY_TYPE_LIST_VIEW,
                                                        TIDY_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
                                   PROP_CELL_RENDERER,
                                   g_param_spec_object ("cell-renderer",
                                                        "Cell Renderer",
                                                        "The cell renderer used by the column",
                                                        TIDY_TYPE_CELL_RENDERER,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_HEADER_RENDERER,
                                   g_param_spec_object ("header-renderer",
                                                        "Header Renderer",
                                                        "The cell renderer used by the column for its header",
                                                        TIDY_TYPE_CELL_RENDERER,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_MAX_WIDTH,
                                   g_param_spec_int ("max-width",
                                                     "Max Width",
                                                     "The maximum width of the column",
                                                     -1, G_MAXINT,
                                                     -1,
                                                     TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_MIN_WIDTH,
                                   g_param_spec_int ("min-width",
                                                     "Min Width",
                                                     "The minimum width of the column",
                                                     -1, G_MAXINT,
                                                     -1,
                                                     TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_WIDTH,
                                   g_param_spec_uint ("width",
                                                      "Width",
                                                      "The current width of the column",
                                                      0, G_MAXUINT,
                                                      0,
                                                      TIDY_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_MODEL_INDEX,
                                   g_param_spec_uint ("model-index",
                                                      "Model Index",
                                                      "The column number of the model bound to the column object",
                                                      0, G_MAXUINT,
                                                      0,
                                                      TIDY_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
                                   PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "The name of the column set by the model",
                                                        NULL,
                                                        TIDY_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_TITLE,
                                   g_param_spec_boolean ("visible",
                                                         "Visible",
                                                         "Whether the column should be visible",
                                                        TRUE,
                                                        TIDY_PARAM_READWRITE));
}

static void
tidy_list_column_init (TidyListColumn *column)
{
  TidyListColumnPrivate *priv;

  column->priv = priv = TIDY_LIST_COLUMN_GET_PRIVATE (column);

  priv->list_view = NULL;

  priv->header = NULL;
  priv->cell = NULL;

  priv->min_width = -1;
  priv->max_width = CLUTTER_UNITS_FROM_DEVICE (75);

  priv->model_id = 0;

  priv->is_visible = TRUE;
}

void
tidy_list_column_set_cell_renderer (TidyListColumn   *column,
                                    TidyCellRenderer *renderer)
{
  TidyListColumnPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_COLUMN (column));
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  priv = column->priv;

  if (priv->cell)
    g_object_unref (priv->cell);

  priv->cell = g_object_ref_sink (renderer);
}

TidyCellRenderer *
tidy_list_column_get_cell_renderer (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), NULL);

  return column->priv->cell;
}

void
tidy_list_column_set_header_renderer (TidyListColumn   *column,
                                      TidyCellRenderer *renderer)
{
  TidyListColumnPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_COLUMN (column));
  g_return_if_fail (TIDY_IS_CELL_RENDERER (renderer));

  priv = column->priv;

  if (priv->header)
    g_object_unref (priv->header);

  priv->header = g_object_ref_sink (renderer);
}

TidyCellRenderer *
tidy_list_column_get_header_renderer (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), NULL);

  return column->priv->header;
}

void
tidy_list_column_set_max_width (TidyListColumn *column,
                                gint            max_width)
{
  tidy_list_column_set_max_widthu (column,
                                   max_width < 0
                                     ? -1
                                     : CLUTTER_UNITS_FROM_DEVICE (max_width));
}

void
tidy_list_column_set_max_widthu (TidyListColumn *column,
                                 ClutterUnit     max_width)
{
  TidyListColumnPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_COLUMN (column));

  priv = column->priv;

  if (max_width < 0)
    max_width = -1;

  g_object_freeze_notify (G_OBJECT (column));

  if (priv->max_width != max_width)
    {
      priv->max_width = max_width;

      if (priv->min_width < priv->max_width)
        {
          priv->min_width = priv->max_width;
          g_object_notify (G_OBJECT (column), "min-width");
        }

      g_object_notify (G_OBJECT (column), "max-width");

      if (priv->list_view && CLUTTER_ACTOR_IS_VISIBLE (priv->list_view))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (priv->list_view));
    }

  g_object_thaw_notify (G_OBJECT (column));
}

gint
tidy_list_column_get_max_width (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), -1);

  return CLUTTER_UNITS_TO_DEVICE (column->priv->max_width);
}

ClutterUnit
tidy_list_column_get_max_widthu (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), -1);

  return column->priv->max_width;
}

void
tidy_list_column_set_min_width (TidyListColumn *column,
                                gint            min_width)
{
  tidy_list_column_set_min_widthu (column,
                                   min_width < 0
                                     ? -1
                                     : CLUTTER_UNITS_FROM_DEVICE (min_width));
}

void
tidy_list_column_set_min_widthu (TidyListColumn *column,
                                 ClutterUnit     min_width)
{
  TidyListColumnPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_COLUMN (column));

  priv = column->priv;

  if (min_width < 0)
    min_width = -1;

  g_object_freeze_notify (G_OBJECT (column));

  if (priv->min_width != min_width)
    {
      priv->min_width = min_width;

      if (priv->max_width < priv->min_width)
        {
          priv->max_width = priv->min_width;
          g_object_notify (G_OBJECT (column), "max-width");
        }

      g_object_notify (G_OBJECT (column), "min-width");

      if (priv->list_view && CLUTTER_ACTOR_IS_VISIBLE (priv->list_view))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (priv->list_view));
    }

  g_object_thaw_notify (G_OBJECT (column));
}

gint
tidy_list_column_get_min_width (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), -1);

  return CLUTTER_UNITS_TO_DEVICE (column->priv->min_width);
}

ClutterUnit
tidy_list_column_get_min_widthu (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), -1);

  return column->priv->min_width;
}

guint
tidy_list_column_get_width (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), 0);

  return CLUTTER_UNITS_TO_DEVICE (tidy_list_column_get_widthu (column));
}

ClutterUnit
tidy_list_column_get_widthu (TidyListColumn *column)
{
  TidyListColumnPrivate *priv;
  GValue value = { 0, };
  ClutterActor *cell;
  ClutterUnit width;

  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), 0);

  priv = column->priv;

  if (!priv->header)
    return priv->min_width;

  if (!priv->list_view)
    return priv->min_width;

  /* get the width from the header */
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, tidy_list_column_get_title (column));

  cell = tidy_cell_renderer_get_cell_actor (priv->header,
                                            TIDY_ACTOR (priv->list_view),
                                            &value,
                                            TIDY_CELL_HEADER,
                                            NULL,
                                            -1, priv->model_id);

  width = clutter_actor_get_widthu (cell);
  clutter_actor_destroy (cell);

  width = CLAMP (width,
                 priv->min_width > 0 ? priv->min_width : width - 1,
                 priv->max_width > 0 ? priv->max_width : width + 1);

  g_value_unset (&value);

  return width;
}

guint
tidy_list_column_get_model_index (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), 0);

  return column->priv->model_id;
}

G_CONST_RETURN gchar *
tidy_list_column_get_title (TidyListColumn *column)
{
  TidyListColumnPrivate *priv;
  ClutterModel *model;

  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), NULL);

  priv = column->priv;
  if (!priv->list_view)
    return NULL;

  model = tidy_list_view_get_model (priv->list_view);
  if (!model)
    return NULL;

  return clutter_model_get_column_name (model, priv->model_id);
}

void
tidy_list_column_set_visible (TidyListColumn *column,
                              gboolean        visible)
{
  TidyListColumnPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_COLUMN (column));

  priv = column->priv;

  if (priv->is_visible != visible)
    {
      priv->is_visible = visible;

      g_object_notify (G_OBJECT (column), "visible");
    }
}

gboolean
tidy_list_column_get_visible (TidyListColumn *column)
{
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN (column), FALSE);

  return column->priv->is_visible;
}
