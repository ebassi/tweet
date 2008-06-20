/* tidy-list-view.c: List actor
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

#include <clutter/clutter.h>
#include <clutter/cogl.h>

#include "tidy-list-view.h"

#include "tidy-adjustment.h"
#include "tidy-debug.h"
#include "tidy-enum-types.h"
#include "tidy-marshal.h"
#include "tidy-private.h"
#include "tidy-scrollable.h"
#include "tidy-stylable.h"

enum
{
  PROP_0,

  PROP_MODEL,
  PROP_SHOW_HEADERS,
  PROP_RULES_HINT,
  PROP_HADJUST,
  PROP_VADJUST
};

enum
{
  ROW_CLICKED,

  LAST_SIGNAL
};

typedef struct _ListHeader
{
  GPtrArray *cells;

  ClutterUnit width;
  ClutterUnit height;
} ListHeader;

typedef struct _ListRow
{
  GPtrArray *cells;

  gint index;

  ClutterUnit y_offset;
  ClutterUnit width;
  ClutterUnit height;
} ListRow;

#define TIDY_LIST_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TIDY_TYPE_LIST_VIEW, TidyListViewPrivate))

struct _TidyListViewPrivate
{
  ClutterModel *model;
  guint n_columns;
  guint n_rows;

  gulong row_added_id;
  gulong row_removed_id;
  gulong row_changed_id;
  gulong sort_changed_id;
  gulong filter_changed_id;

  /* holds the ListColumns */
  GList *columns;

  ListHeader *header;

  /* holds the ListRows */
  GList *rows;

  ClutterActorBox allocation;

  ClutterUnit last_row_y;

  guint show_headers : 1;
  guint rules_hint   : 1;
  
  TidyAdjustment *hadjustment;
  TidyAdjustment *vadjustment;
};

static guint view_signals[LAST_SIGNAL] = { 0, };

static ClutterColor default_hint_color = { 0xf0, 0xb3, 0x78, 0xff };
static guint        default_v_padding  = 2;
static guint        default_h_padding  = 2;

static void tidy_stylable_iface_init (TidyStylableIface *iface);

static void
tidy_list_view_refresh_hadjustment (TidyListView *view);

static void
tidy_list_view_refresh_vadjustment (TidyListView *view);

static void
tidy_list_view_set_adjustments (TidyScrollable *scrollable,
                                TidyAdjustment *hadjustment,
                                TidyAdjustment *vadjustment);

static void
tidy_list_view_get_adjustments (TidyScrollable *scrollable,
                                TidyAdjustment **hadjustment,
                                TidyAdjustment **vadjustment);

static void tidy_scrollable_iface_init (TidyScrollableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (TidyListView, tidy_list_view, TIDY_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (TIDY_TYPE_STYLABLE,
                                                tidy_stylable_iface_init)
                         G_IMPLEMENT_INTERFACE (TIDY_TYPE_SCROLLABLE,
                                                tidy_scrollable_iface_init));

static void
clear_header (ListHeader *header)
{
  gint i;

  if (G_UNLIKELY (header == NULL))
    return;

  if (header->cells)
    {
      for (i = 0; i < header->cells->len; i++)
        clutter_actor_unparent (g_ptr_array_index (header->cells, i));

      g_ptr_array_free (header->cells, TRUE);
    }

  g_slice_free (ListHeader, header);
}

static void
clear_row (ListRow *row)
{
  gint i;

  if (G_UNLIKELY (row == NULL))
    return;

  if (row->cells)
    {
      for (i = 0; i < row->cells->len; i++)
        clutter_actor_unparent (g_ptr_array_index (row->cells, i));

      g_ptr_array_free (row->cells, TRUE);
    }

  g_slice_free (ListRow, row);
}

static void
clear_layout (TidyListView *view,
              gboolean      clear_headers)
{
  TidyListViewPrivate *priv = view->priv;

  if (clear_headers)
    {
      clear_header (priv->header);
      priv->header = NULL;
    }

  g_list_foreach (priv->rows, (GFunc) clear_row, NULL);
  g_list_free (priv->rows);
  priv->rows = NULL;
}

static void
tidy_list_view_finalize (GObject *gobject)
{
  G_OBJECT_CLASS (tidy_list_view_parent_class)->finalize (gobject);
}

static void
tidy_list_view_dispose (GObject *gobject)
{
  TidyListViewPrivate *priv = TIDY_LIST_VIEW_GET_PRIVATE (gobject);

  clear_layout (TIDY_LIST_VIEW (gobject), TRUE);

  if (priv->model)
    {
      g_signal_handler_disconnect (priv->model, priv->row_added_id);
      g_signal_handler_disconnect (priv->model, priv->row_changed_id);
      g_signal_handler_disconnect (priv->model, priv->row_removed_id);

      g_object_unref (priv->model);
      priv->model = NULL;
    }

  if (priv->columns)
    {
      g_list_foreach (priv->columns, (GFunc) g_object_unref, NULL);
      g_list_free (priv->columns);
      priv->columns = NULL;
    }

  G_OBJECT_CLASS (tidy_list_view_parent_class)->dispose (gobject);
}

static void
tidy_list_view_set_property (GObject      *gobject,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  TidyListView *view = TIDY_LIST_VIEW (gobject);

  switch (prop_id)
    {
    case PROP_MODEL:
      tidy_list_view_set_model (view, g_value_get_object (value));
      break;
    case PROP_SHOW_HEADERS:
      tidy_list_view_set_show_headers (view, g_value_get_boolean (value));
      break;
    case PROP_RULES_HINT:
      tidy_list_view_set_rules_hint (view, g_value_get_boolean (value));
      break;
    case PROP_HADJUST :
      tidy_list_view_set_adjustments (TIDY_SCROLLABLE (gobject),
                                      g_value_get_object (value),
                                      view->priv->vadjustment);
      break;
    case PROP_VADJUST :
      tidy_list_view_set_adjustments (TIDY_SCROLLABLE (gobject),
                                      view->priv->hadjustment,
                                      g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_list_view_get_property (GObject    *gobject,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  TidyAdjustment *adjustment;

  TidyListView *view = TIDY_LIST_VIEW (gobject);

  switch (prop_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, view->priv->model);
      break;
    case PROP_SHOW_HEADERS:
      g_value_set_boolean (value, view->priv->show_headers);
      break;
    case PROP_RULES_HINT:
      g_value_set_boolean (value, view->priv->rules_hint);
      break;
    case PROP_HADJUST :
      tidy_list_view_get_adjustments (TIDY_SCROLLABLE (gobject),
                                      &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;
    case PROP_VADJUST :
      tidy_list_view_get_adjustments (TIDY_SCROLLABLE (gobject),
                                      NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static gint
tidy_list_view_visible_columns (TidyListView *view)
{
  GList *c;
  
  TidyListViewPrivate *priv = view->priv;
  gint visible = 0;
  
  for (c = priv->columns; c; c = c->next)
    if (tidy_list_column_get_visible (TIDY_LIST_COLUMN (c->data)))
      visible ++;
  
  return visible;
}

static void
append_row_layout (TidyListView     *view,
                   ClutterModelIter *iter)
{
  TidyListViewPrivate *priv = view->priv;
  ListRow *row_info;
  guint n_columns, nv_columns;
  ClutterUnit width;
  ClutterUnit x_offset, y_offset, cell_height;
  gint h_padding, v_padding;
  gint i;
  GList *l;

  h_padding = default_h_padding;
  v_padding = default_v_padding;

  tidy_stylable_get (TIDY_STYLABLE (view),
                     "h-padding", &h_padding,
                     "v-padding", &v_padding,
                     NULL);

  width = priv->allocation.x2 - priv->allocation.x1;
  if (width <= 0)
    width = clutter_actor_get_widthu (CLUTTER_ACTOR (view));

  n_columns = g_list_length (priv->columns);

  x_offset = y_offset = cell_height = 0;

  y_offset = priv->last_row_y;
  y_offset += CLUTTER_UNITS_FROM_DEVICE (v_padding);

  row_info = g_slice_new (ListRow);
  row_info->cells = g_ptr_array_sized_new (n_columns);
  row_info->index = clutter_model_iter_get_row (iter);
  row_info->y_offset = y_offset;
  row_info->width = 0;
  row_info->height = 0;

  nv_columns = tidy_list_view_visible_columns (view);

  for (l = priv->columns, i = 0; l != NULL; l = l->next, i++)
    {
      TidyListColumn *column = l->data;
      TidyCellRenderer *renderer;
      ClutterGeometry size = { 0, };
      GValue value = { 0, };
      ClutterActor *cell;
      ClutterUnit column_width;
      guint model_id;
      TidyCellState state;

      if (!tidy_list_column_get_visible (column))
        continue;

      model_id = tidy_list_column_get_model_index (column);

      if (model_id == clutter_model_get_sorting_column (priv->model))
        state = TIDY_CELL_SORTING;
      else
        state = TIDY_CELL_NORMAL;

      clutter_model_iter_get_value (iter, model_id, &value);

      column_width = tidy_list_column_get_widthu (column);
      column_width = MAX ((width / nv_columns), column_width);

      /* provide a geometry for the cell */
      size.x = CLUTTER_UNITS_TO_DEVICE (x_offset);
      size.y = CLUTTER_UNITS_TO_DEVICE (y_offset);
      size.width = CLUTTER_UNITS_TO_DEVICE (column_width);
      size.height = (cell_height > 0
                     ? CLUTTER_UNITS_TO_DEVICE (cell_height)
                     : -1);

      renderer = tidy_list_column_get_cell_renderer (column);
      cell = tidy_cell_renderer_get_cell_actor (renderer,
                                                TIDY_ACTOR (view),
                                                &value,
                                                state, &size,
                                                row_info->index, i);

      g_value_unset (&value);

      g_ptr_array_add (row_info->cells, cell);
      clutter_actor_set_parent (cell, CLUTTER_ACTOR (view));
      clutter_actor_set_positionu (cell, x_offset, y_offset);
      clutter_actor_set_widthu (cell, column_width);
      clutter_actor_show (cell);

      x_offset += column_width;
      x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);

      cell_height = MAX (cell_height, clutter_actor_get_heightu (cell));
    }

  row_info->width = x_offset;
  row_info->height = cell_height;

  priv->last_row_y = y_offset;
  priv->rows = g_list_append (priv->rows, row_info);

  /* store the layout size */
  priv->allocation.x2 = priv->allocation.x1 + width;
  priv->allocation.y2 = priv->allocation.y1 + y_offset;

  /* Adjust the adjustments */
  if (priv->hadjustment)
    tidy_list_view_refresh_hadjustment (view);

  if (priv->vadjustment)
    tidy_list_view_refresh_vadjustment (view);
}

static void
prepend_row_layout (TidyListView     *view,
                    ClutterModelIter *iter)
{
  TidyListViewPrivate *priv = view->priv;
  ListRow *row_info;
  guint n_columns, nv_columns;
  ClutterUnit width;
  ClutterUnit x_offset, y_offset, cell_height;
  ClutterUnit next_row_offset;
  gint h_padding, v_padding;
  gint i;
  GList *l;

  h_padding = default_h_padding;
  v_padding = default_v_padding;

  tidy_stylable_get (TIDY_STYLABLE (view),
                     "h-padding", &h_padding,
                     "v-padding", &v_padding,
                     NULL);

  width = priv->allocation.x2 - priv->allocation.x1;
  if (width <= 0)
    width = clutter_actor_get_widthu (CLUTTER_ACTOR (view));

  n_columns = g_list_length (priv->columns);

  x_offset = y_offset = cell_height = 0;

  if (priv->show_headers)
    {
      y_offset += priv->header->height;
      y_offset += CLUTTER_UNITS_FROM_DEVICE (v_padding);
    }

  row_info = g_slice_new (ListRow);
  row_info->cells = g_ptr_array_sized_new (n_columns);
  row_info->index = clutter_model_iter_get_row (iter);
  row_info->y_offset = y_offset;
  row_info->width = 0;
  row_info->height = 0;

  nv_columns = tidy_list_view_visible_columns (view);

  for (l = priv->columns, i = 0; l != NULL; l = l->next, i++)
    {
      TidyListColumn *column = l->data;
      TidyCellRenderer *renderer;
      ClutterGeometry size = { 0, };
      GValue value = { 0, };
      ClutterActor *cell;
      ClutterUnit column_width;
      guint model_id;
      TidyCellState state;

      if (!tidy_list_column_get_visible (column))
        continue;

      model_id = tidy_list_column_get_model_index (column);

      if (model_id == clutter_model_get_sorting_column (priv->model))
        state = TIDY_CELL_SORTING;
      else
        state = TIDY_CELL_NORMAL;

      clutter_model_iter_get_value (iter, model_id, &value);

      column_width = tidy_list_column_get_widthu (column);
      column_width = MAX ((width / nv_columns), column_width);

      /* provide a geometry for the cell */
      size.x = CLUTTER_UNITS_TO_DEVICE (x_offset);
      size.y = CLUTTER_UNITS_TO_DEVICE (y_offset);
      size.width = CLUTTER_UNITS_TO_DEVICE (column_width);
      size.height = (cell_height > 0
                     ? CLUTTER_UNITS_TO_DEVICE (cell_height)
                     : -1);

      renderer = tidy_list_column_get_cell_renderer (column);
      cell = tidy_cell_renderer_get_cell_actor (renderer,
                                                TIDY_ACTOR (view),
                                                &value,
                                                state, &size,
                                                row_info->index, i);

      g_value_unset (&value);

      g_ptr_array_add (row_info->cells, cell);
      clutter_actor_set_parent (cell, CLUTTER_ACTOR (view));
      clutter_actor_set_positionu (cell, x_offset, y_offset);
      clutter_actor_set_widthu (cell, column_width);
      clutter_actor_show (cell);

      cell_height = MAX (cell_height, clutter_actor_get_heightu (cell));

      x_offset += column_width;
      x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);
    }

  row_info->width = x_offset;
  row_info->height = cell_height;

  next_row_offset = row_info->y_offset
                  + cell_height
                  + CLUTTER_UNITS_FROM_DEVICE (v_padding);

  for (l = priv->rows; l != NULL; l = l->next)
    {
      ListRow *r = l->data;
      GList *c;

      x_offset = 0;

      r->y_offset = next_row_offset;

      for (c = priv->columns, i = 0; c != NULL; c = c->next, i++)
        {
          TidyListColumn *column = c->data;
          ClutterUnit column_width;
          ClutterActor *cell;

          if (!tidy_list_column_get_visible (column))
            continue;

          column_width = tidy_list_column_get_widthu (column);
          column_width = MAX ((width / nv_columns), column_width);

          cell = g_ptr_array_index (r->cells, i);

          clutter_actor_set_positionu (cell, x_offset, next_row_offset);

          x_offset += column_width;
          x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);
        }

      next_row_offset = r->y_offset
                      + r->height
                      + CLUTTER_UNITS_FROM_DEVICE (v_padding);
    }

  priv->last_row_y = next_row_offset;
  priv->rows = g_list_prepend (priv->rows, row_info);

  /* store the layout size */
  priv->allocation.x2 = priv->allocation.x1 + width;
  priv->allocation.y2 = priv->allocation.y2
                      + row_info->height
                      + CLUTTER_UNITS_FROM_DEVICE (v_padding);

  /* Adjust the adjustments */
  if (priv->hadjustment)
    tidy_list_view_refresh_hadjustment (view);

  if (priv->vadjustment)
    tidy_list_view_refresh_vadjustment (view);
}

/* if the saved allocation is set then the layout will use that; if
 * it's not set, it will be set to the maximum extent of the children
 */
static void
ensure_layout (TidyListView *view)
{
  TidyListViewPrivate *priv = view->priv;
  ClutterActor *actor = CLUTTER_ACTOR (view);
  ClutterUnit width, header_height;
  ClutterUnit x_offset, y_offset;
  guint v_padding, v_paddingu, h_padding;
  GList *l;
  gint n_columns, nv_columns, n_rows;
  gint row;
  gint i;

  if (priv->rows)
    return;

  h_padding = default_h_padding;
  v_padding = default_v_padding;

  tidy_stylable_get (TIDY_STYLABLE (view),
                     "h-padding", &h_padding,
                     "v-padding", &v_padding,
                     NULL);
  
  v_paddingu = CLUTTER_UNITS_FROM_DEVICE (v_padding);

  width = priv->allocation.x2 - priv->allocation.x1;

  if (width <= 0)
    width = clutter_actor_get_widthu (actor);

  n_columns = g_list_length (priv->columns);
  nv_columns = tidy_list_view_visible_columns (view);

  x_offset = y_offset = header_height = 0;

  if (priv->show_headers)
    {
      if (!priv->header)
        {
          ListHeader *header;
    
          header = g_slice_new (ListHeader);
          header->cells = g_ptr_array_sized_new (n_columns);
          header->width = 0;
          header->height = 0;
    
          for (l = priv->columns, i = 0; l != NULL; l = l->next, i++)
            {
              TidyListColumn *column = l->data;
              TidyCellRenderer *renderer;
              ClutterGeometry size = { 0, };
              GValue title = { 0, };
              ClutterUnit column_width;
              ClutterActor *cell;
    
              if (!tidy_list_column_get_visible (column))
                continue;
    
              column_width = tidy_list_column_get_widthu (column);
              column_width = MAX ((width / nv_columns), column_width);

              /* ClutterGeometry is pixels based; maybe we should
               * switch to ClutterActorBox
               */
              size.x = CLUTTER_UNITS_TO_DEVICE (x_offset);
              size.y = CLUTTER_UNITS_TO_DEVICE (y_offset);
              size.width = CLUTTER_UNITS_TO_DEVICE (column_width);
              size.height = (header_height > 0
                             ? CLUTTER_UNITS_TO_DEVICE (header_height)
                             : -1);
    
              g_value_init (&title, G_TYPE_STRING);
              g_value_set_string (&title, tidy_list_column_get_title (column));
    
              renderer = tidy_list_column_get_header_renderer (column);
              cell = tidy_cell_renderer_get_cell_actor (renderer,
                                                        TIDY_ACTOR (actor),
                                                        &title,
                                                        TIDY_CELL_HEADER, &size,
                                                        -1, i);
    
              g_value_unset (&title);
    
              g_ptr_array_add (header->cells, cell);

              clutter_actor_set_parent (cell, actor);
              clutter_actor_set_positionu (cell, x_offset, y_offset);
              clutter_actor_set_widthu (cell, column_width);
              clutter_actor_show (cell);

              x_offset += column_width;
              x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);

              header_height =
                MAX (header_height, clutter_actor_get_heightu (cell));
            }
    
          header->width = x_offset;
          header->height = header_height;
    
          priv->header = header;
    
          y_offset += priv->header->height;
          y_offset += v_paddingu;
    
          x_offset = 0;
        }
      else
        {
          y_offset += priv->header->height;
          y_offset += v_paddingu;

          x_offset = 0;
        }
    }

  n_rows = priv->model ? clutter_model_get_n_rows (priv->model) : 0;

  for (row = 0; row < n_rows; row++)
    {
      ListRow *row_info;
      ClutterModelIter *iter;
      ClutterUnit cell_height = 0;

      iter = clutter_model_get_iter_at_row (priv->model, row);
      if (!iter)
        continue;

      row_info = g_slice_new (ListRow);
      row_info->cells = g_ptr_array_sized_new (n_columns);
      row_info->index = row;
      row_info->y_offset = y_offset;
      row_info->width = 0;
      row_info->height = 0;

      for (l = priv->columns, i = 0; l != NULL; l = l->next, i++)
        {
          TidyListColumn *column = l->data;
          TidyCellRenderer *renderer;
          ClutterGeometry size = { 0, };
          GValue value = { 0, };
          ClutterActor *cell;
          ClutterUnit column_width;
          guint model_id;
          TidyCellState state;

          if (!tidy_list_column_get_visible (column))
            continue;

          model_id = tidy_list_column_get_model_index (column);

          if (model_id == clutter_model_get_sorting_column (priv->model))
            state = TIDY_CELL_SORTING;
          else
            state = TIDY_CELL_NORMAL;

          clutter_model_iter_get_value (iter, model_id, &value);

          column_width = tidy_list_column_get_widthu (column);
          column_width = MAX ((width / nv_columns), column_width);

          /* see above, re: ClutterGeometry */
          size.x = CLUTTER_UNITS_TO_DEVICE (x_offset);
          size.y = CLUTTER_UNITS_TO_DEVICE (y_offset);
          size.width = CLUTTER_UNITS_TO_DEVICE (column_width);
          size.height = (cell_height > 0
                         ? CLUTTER_UNITS_TO_DEVICE (cell_height)
                         : -1);

          renderer = tidy_list_column_get_cell_renderer (column);
          cell = tidy_cell_renderer_get_cell_actor (renderer,
                                                    TIDY_ACTOR (actor),
                                                    &value,
                                                    state, &size,
                                                    row, i);

          g_value_unset (&value);

          g_ptr_array_add (row_info->cells, cell);
          clutter_actor_set_parent (cell, actor);
          clutter_actor_set_positionu (cell, x_offset, y_offset);
          clutter_actor_set_widthu (cell, column_width);
          clutter_actor_show (cell);

          x_offset += column_width;
          x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);

          cell_height = MAX (cell_height, clutter_actor_get_heightu (cell));
        }

      row_info->width = x_offset;
      row_info->height = cell_height;

      g_object_unref (iter);

      y_offset += cell_height;
      y_offset += v_paddingu;

      x_offset = 0;

      priv->rows = g_list_prepend (priv->rows, row_info);
    }

  priv->last_row_y = y_offset;

  priv->rows = g_list_reverse (priv->rows);

  /* store the layout size */
  priv->allocation.x2 = priv->allocation.x1 + width;
  priv->allocation.y2 = priv->allocation.y1 + y_offset;

  /* Adjust the adjustments */
  if (priv->hadjustment)
    tidy_list_view_refresh_hadjustment (view);

  if (priv->vadjustment)
    tidy_list_view_refresh_vadjustment (view);
}

static void
on_row_added (ClutterModel     *model,
              ClutterModelIter *iter,
              TidyListView     *list_view)
{
  ClutterModelIter *copy = NULL;
  guint row;

  /* the row is guaranteed to already be inside the model; we need to
   * check if it's the last one as well. XXX this is a hack based on
   * the fact that we know how the implementation of the model works.
   *
   * we need a ClutterModelIter::copy() function so we can copy the
   * passed iterator, advance the copy and check if it's the last
   * row.
   */
  row = clutter_model_iter_get_row (iter);
  copy = clutter_model_get_iter_at_row (model, row);

  /* if the row was appended, then there's no need to rebuild the
   * entire layout of the list view
   */
  if (copy)
    {
      if (clutter_model_iter_is_last (copy))
        {
          append_row_layout (list_view, iter);
          goto done;
        }
    }

  if (copy)
    {
      if (clutter_model_iter_is_first (copy))
        {
          prepend_row_layout (list_view, iter);
          goto done;
        }
      else
        {
          copy = clutter_model_iter_prev (copy);
          if (clutter_model_iter_is_first (copy))
            {
              prepend_row_layout (list_view, iter);
              goto done;
            }
        }
    }

  clear_layout (list_view, FALSE);
  ensure_layout (list_view);

done:
  if (copy)
    g_object_unref (copy);

  if (CLUTTER_ACTOR_IS_VISIBLE (list_view))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (list_view));
}

static void
on_row_removed (ClutterModel     *model,
                ClutterModelIter *iter,
                TidyListView     *list_view)
{
  clear_layout (list_view, FALSE);
  ensure_layout (list_view);
}

static void
on_row_changed (ClutterModel     *model,
                ClutterModelIter *iter,
                TidyListView     *view)
{
  gint nv_columns, i, row;
  ClutterUnit width;
  ListRow *row_info;
  GList *l;
  TidyListViewPrivate *priv = view->priv;
  ClutterActor *actor = CLUTTER_ACTOR (view);
  ClutterUnit cell_height = 0;
  gint h_padding;

  h_padding = default_h_padding;

  tidy_stylable_get (TIDY_STYLABLE (view), "h-padding", &h_padding, NULL);

  /* Check if the new row will occupy the space of the old row - if so,
   * don't relayout the entire list
   */

  row = clutter_model_iter_get_row (iter);
  row_info = g_list_nth_data (priv->rows, row);

  if (row_info)
    {
      ClutterUnit x_offset;

      width = priv->allocation.x2 - priv->allocation.x1;
      if (width <= 0)
        width = clutter_actor_get_widthu (actor);

      x_offset = 0;

      nv_columns = tidy_list_view_visible_columns (view);

      for (l = priv->columns, i = 0; l != NULL; l = l->next, i++)
        {
          TidyListColumn *column = l->data;
          TidyCellRenderer *renderer;
          ClutterGeometry size = { 0, };
          GValue value = { 0, };
          ClutterActor *cell, *old_cell;
          ClutterUnit column_width;
          guint model_id;
          TidyCellState state;

          if (!tidy_list_column_get_visible (column))
            continue;

          if (row_info->cells->len <= i)
            break;

          /* Replace cell */
          old_cell = (ClutterActor *) g_ptr_array_index (row_info->cells, i);
          clutter_actor_hide (old_cell);

          model_id = tidy_list_column_get_model_index (column);

          if (model_id == clutter_model_get_sorting_column (priv->model))
            state = TIDY_CELL_SORTING;
          else
            state = TIDY_CELL_NORMAL;

          clutter_model_iter_get_value (iter, model_id, &value);

          column_width = tidy_list_column_get_widthu (column);
          column_width = MAX ((width / nv_columns), column_width);

          size.width = CLUTTER_UNITS_TO_DEVICE (column_width);
          size.height = (cell_height > 0
                         ? CLUTTER_UNITS_TO_DEVICE (cell_height)
                         : -1);

          renderer = tidy_list_column_get_cell_renderer (column);
          cell = tidy_cell_renderer_get_cell_actor (renderer,
                                                    TIDY_ACTOR (actor),
                                                    &value,
                                                    state, &size,
                                                    row, i);
          g_value_unset (&value);

          g_ptr_array_index (row_info->cells, i) = cell;
          clutter_actor_set_parent (cell, actor);
          clutter_actor_set_positionu (cell, x_offset, row_info->y_offset);
          clutter_actor_set_widthu (cell, column_width);
          clutter_actor_show (cell);

          cell_height = MAX (cell_height, clutter_actor_get_heightu (cell));

          x_offset += column_width;
          x_offset += CLUTTER_UNITS_FROM_DEVICE (h_padding);

          /* Remove old cell */
          clutter_actor_unparent (old_cell);
        }

      /* If row height is unchanged, new cells fit within the old cell space */
      if ((!l) && (cell_height == row_info->height))
        {
          if (CLUTTER_ACTOR_IS_VISIBLE (view))
            clutter_actor_queue_redraw (CLUTTER_ACTOR (view));

          return;
        }
    }

  /* Relayout */
  clear_layout (view, FALSE);
  ensure_layout (view);
}

static void
on_sort_changed (ClutterModel *model,
                 TidyListView *list_view)
{
  clear_layout (list_view, FALSE);
  ensure_layout (list_view);
}

static void
on_filter_changed (ClutterModel *model,
                   TidyListView *list_view)
{
  clear_layout (list_view, FALSE);
  ensure_layout (list_view);
}

static void
tidy_list_view_request_coords (ClutterActor    *actor,
                               ClutterActorBox *box)
{
  TidyListView *list_view = TIDY_LIST_VIEW (actor);
  TidyListViewPrivate *priv = list_view->priv;

  priv->allocation = *box;

  clear_layout (list_view, TRUE);
  ensure_layout (list_view);

  CLUTTER_ACTOR_CLASS (tidy_list_view_parent_class)->request_coords (actor, box);
}

static void
tidy_list_view_query_coords (ClutterActor    *actor,
                             ClutterActorBox *box)
{
  TidyListViewPrivate *priv = TIDY_LIST_VIEW (actor)->priv;

  *box = priv->allocation;
}

static void
tidy_list_view_paint (ClutterActor *actor)
{
  TidyListViewPrivate *priv;
  ClutterColor *hint_color = NULL;
  GList *l;
  ClutterUnit i, x, y, width, height;
  guint h_padding, v_padding;
  ClutterUnit h_paddingu, v_paddingu;
  gboolean has_clip;

  h_padding = default_h_padding;
  v_padding = default_v_padding;

  tidy_stylable_get (TIDY_STYLABLE (actor),
                     "h-padding", &h_padding,
                     "v-padding", &v_padding,
                     "row-hint-color", &hint_color,
                     NULL);

  if (!hint_color)
    hint_color = &default_hint_color;

  h_paddingu = CLUTTER_UNITS_FROM_DEVICE (h_padding);
  v_paddingu = CLUTTER_UNITS_FROM_DEVICE (v_padding);

  hint_color->alpha = ((hint_color->alpha *
                       (gint)clutter_actor_get_opacity (actor)) / 255);

  priv = TIDY_LIST_VIEW (actor)->priv;

  cogl_push_matrix ();

  /* Check clipping rectangle to stop unnecessary drawing */
  if ((has_clip = clutter_actor_has_clip (actor)))
    clutter_actor_get_clipu (actor, &x, &y, &width, &height);

  /* Set horizontal offset */
  if (priv->hadjustment)
    {
      ClutterFixed hoffset = tidy_adjustment_get_valuex (priv->hadjustment);

      cogl_translatex (-hoffset, 0, 0);
      x += CLUTTER_UNITS_FROM_FIXED (hoffset);
    }
  
  /* Save position and set vertical offset */
  cogl_push_matrix ();
  if (priv->vadjustment)
    {
      ClutterFixed voffset = tidy_adjustment_get_valuex (priv->vadjustment);

      cogl_translatex (0, -voffset, 0);
      y += CLUTTER_UNITS_FROM_FIXED (voffset);
    }

  /* rows painted first first */
  for (l = priv->rows; l != NULL; l = l->next)
    {
      ListRow *row = l->data;
      gint i;
      ClutterUnit row_offset = row->y_offset;
      ClutterUnit row_width = row->width;
      ClutterUnit row_height = row->height;

      /* TODO: Skip columns that aren't visible */
      if (has_clip &&
          (((row_offset + row_height + v_paddingu) < y) ||
           ((row_offset - v_paddingu / 2) > y + height)))
        continue;

      /* hinting */
      if (G_LIKELY (priv->rules_hint) && (row->index % 2))
        {
          cogl_enable (CGL_ENABLE_BLEND);
          cogl_color (hint_color);
          cogl_rectangle (0,
                          CLUTTER_UNITS_TO_DEVICE (row_offset - (v_paddingu / 2)),
                          CLUTTER_UNITS_TO_DEVICE (row_width),
                          CLUTTER_UNITS_TO_DEVICE (row_height + (v_paddingu / 2)));
        }

      for (i = 0; i < row->cells->len; i++)
        {
          ClutterActor *cell = g_ptr_array_index (row->cells, i);

          if (CLUTTER_ACTOR_IS_VISIBLE (cell))
            clutter_actor_paint (cell);
        }
    }

  /* Restore vertical position */
  cogl_pop_matrix ();

  /* paint the headers */
  if (G_LIKELY (priv->show_headers) && priv->header)
    {
      GPtrArray *headers = priv->header->cells;

      for (i = 0; i < headers->len; i++)
        {
          ClutterActor *cell = g_ptr_array_index (headers, i);

          if (CLUTTER_ACTOR_IS_VISIBLE (cell))
            clutter_actor_paint (cell);
        }
    }

  cogl_pop_matrix ();

  if (hint_color != &default_hint_color)
    clutter_color_free (hint_color);
}

static gboolean
tidy_list_view_scroll (ClutterActor       *actor,
                       ClutterScrollEvent *event)
{
  TidyListView *list_view = TIDY_LIST_VIEW (actor);
  ClutterFixed value;
  ClutterFixed lower, upper;
  ClutterFixed page_increment;

  if (!list_view->priv->vadjustment)
    return FALSE;

  tidy_adjustment_get_valuesx (list_view->priv->vadjustment,
                               &value,
                               &lower, &upper,
                               NULL,
                               &page_increment,
                               NULL);

  switch (event->direction)
    {
    case CLUTTER_SCROLL_UP:
      value -= page_increment;
      value = MAX (lower, value);
      break;

    case CLUTTER_SCROLL_DOWN:
      value += page_increment;
      value = MIN (upper, value);
      break;

    default:
      return FALSE;
    }

  tidy_adjustment_set_valuex (list_view->priv->vadjustment, value);

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_redraw (actor);

  return TRUE;
}

static gboolean
tidy_list_view_key_press (ClutterActor    *actor,
                          ClutterKeyEvent *event)
{
  TidyListView *list_view = TIDY_LIST_VIEW (actor);
  TidyListViewPrivate *priv = list_view->priv;
  ClutterFixed value;
  ClutterFixed lower, upper;
  ClutterFixed page_increment;
  gint n_visible_rows;
  ClutterUnit page_height;
  ClutterUnit x, y, width, height;
  guint h_padding, v_padding;
  ClutterUnit h_paddingu, v_paddingu;
  gboolean has_clip;
  GList *l;

  if (!priv->vadjustment)
    return FALSE;

  tidy_adjustment_get_valuesx (priv->vadjustment,
                               &value,
                               &lower, &upper,
                               NULL,
                               &page_increment,
                               NULL);


  h_padding = default_h_padding;
  v_padding = default_v_padding;

  tidy_stylable_get (TIDY_STYLABLE (actor),
                     "h-padding", &h_padding,
                     "v-padding", &v_padding,
                     NULL);

  h_paddingu = CLUTTER_UNITS_FROM_DEVICE (h_padding);
  v_paddingu = CLUTTER_UNITS_FROM_DEVICE (v_padding);

  /* Check clipping rectangle to compute the visible rows */
  if ((has_clip = clutter_actor_has_clip (actor)))
    clutter_actor_get_clipu (actor, &x, &y, &width, &height);
  else
    {
      x = y = width = height = 0;

      page_height = 0;
    }

  y += CLUTTER_UNITS_FROM_FIXED (value);

  /* rows painted first first */
  for (l = priv->rows; l != NULL; l = l->next)
    {
      ListRow *row = l->data;
      ClutterUnit row_offset = row->y_offset;
      ClutterUnit row_height = row->height;

      /* leading rows */
      if (has_clip && ((row_offset + row_height + v_paddingu) < y))
        continue;

      /* trailing rows */
      if (has_clip && ((row_offset - v_paddingu / 2) > y + height))
        break;;

      n_visible_rows += 1;
      page_height += row_height;
    }

  switch (event->keyval)
    {
    case CLUTTER_Home:
    case CLUTTER_KP_Home:
      value = 0;
      break;

    case CLUTTER_Page_Up:
    case CLUTTER_KP_Page_Up:
      value -= page_height;
      value = MAX (lower, value);
      break;

    case CLUTTER_Page_Down:
    case CLUTTER_KP_Page_Down:
      value += page_height;
      value = MIN (upper, value);
      break;

    case CLUTTER_End:
    case CLUTTER_KP_End:
      value = upper;
      break;

    default:
      return FALSE;
    }

  tidy_adjustment_set_valuex (priv->vadjustment, value);

  if (CLUTTER_ACTOR_IS_VISIBLE (actor))
    clutter_actor_queue_redraw (actor);

  return TRUE;
}

static gboolean
tidy_list_view_button_press (ClutterActor       *actor,
                             ClutterButtonEvent *event)
{
  TidyListView *list_view = TIDY_LIST_VIEW (actor);
  ClutterModel *model = NULL;
  ClutterModelIter *iter;
  gint x, y;
  guint row;

  model = list_view->priv->model;
  if (!model)
    return FALSE;

  clutter_event_get_coords ((ClutterEvent *) event, &x, &y);
  row = tidy_list_view_get_row_at_pos (list_view, x, y);
  if (row < 0)
    return FALSE;

  iter = clutter_model_get_iter_at_row (model, row);
  if (!iter)
    return FALSE;

  g_signal_emit (list_view, view_signals[ROW_CLICKED], 0, model, iter);
  g_object_unref (iter);

  return TRUE;
}

static TidyListColumn *
tidy_list_view_real_create_column (TidyListView *list_view,
                                   guint         model_id)
{
  return NULL;
}

static void
tidy_list_view_class_init (TidyListViewClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyListViewPrivate));

  klass->create_column = tidy_list_view_real_create_column;

  gobject_class->set_property = tidy_list_view_set_property;
  gobject_class->get_property = tidy_list_view_get_property;
  gobject_class->dispose = tidy_list_view_dispose;
  gobject_class->finalize = tidy_list_view_finalize;

  actor_class->request_coords = tidy_list_view_request_coords;
  actor_class->query_coords = tidy_list_view_query_coords;
  actor_class->paint = tidy_list_view_paint;
  actor_class->button_press_event = tidy_list_view_button_press;
  actor_class->scroll_event = tidy_list_view_scroll;
  actor_class->key_press_event = tidy_list_view_key_press;

  g_object_class_install_property (gobject_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
                                                        "Model",
                                                        "The model to be displayed",
                                                        CLUTTER_TYPE_MODEL,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_SHOW_HEADERS,
                                   g_param_spec_boolean ("show-headers",
                                                         "Show Headers",
                                                         "Whether the column headers should be displayed",
                                                         TRUE,
                                                         TIDY_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_RULES_HINT,
                                   g_param_spec_boolean ("rules-hint",
                                                         "Rules Hint",
                                                         "Whether row hinting should be used",
                                                         TRUE,
                                                         TIDY_PARAM_READWRITE));

  g_object_class_override_property (gobject_class,
                                    PROP_HADJUST,
                                    "hadjustment");

  g_object_class_override_property (gobject_class,
                                    PROP_VADJUST,
                                    "vadjustment");

  view_signals[ROW_CLICKED] =
    g_signal_new (I_("row-clicked"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (TidyListViewClass, row_clicked),
                  NULL, NULL,
                  _tidy_marshal_VOID__OBJECT_OBJECT,
                  G_TYPE_NONE, 2,
                  CLUTTER_TYPE_MODEL,
                  CLUTTER_TYPE_MODEL_ITER);
}

static void
tidy_stylable_iface_init (TidyStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      pspec = g_param_spec_boxed ("row-hint-color",
                                  "Row Hint Color",
                                  "Color of the row hinting",
                                  CLUTTER_TYPE_COLOR,
                                  G_PARAM_READWRITE);
      tidy_stylable_iface_install_property (iface, TIDY_TYPE_LIST_VIEW, pspec);

      pspec = g_param_spec_uint ("h-padding",
                                 "Horizontal Padding",
                                 "Padding between cells in a row, in px",
                                 0, G_MAXUINT, 2,
                                 G_PARAM_READWRITE);
      tidy_stylable_iface_install_property (iface, TIDY_TYPE_LIST_VIEW, pspec);

      pspec = g_param_spec_uint ("v-padding",
                                 "Vertical Padding",
                                 "Padding between rows, in px",
                                 0, G_MAXUINT, 2,
                                 G_PARAM_READWRITE);
      tidy_stylable_iface_install_property (iface, TIDY_TYPE_LIST_VIEW, pspec);
    }
}

static void
hadjustment_value_notify_cb (TidyAdjustment *adjustment,
                             GParamSpec     *pspec,
                             gpointer        user_data)
{
  clutter_actor_queue_redraw (CLUTTER_ACTOR (user_data));
}

static void
vadjustment_value_notify_cb (TidyAdjustment *adjustment,
                             GParamSpec     *pspec,
                             gpointer        user_data)
{
  clutter_actor_queue_redraw (CLUTTER_ACTOR (user_data));
}

static void
tidy_list_view_set_adjustments (TidyScrollable *scrollable,
                                TidyAdjustment *hadjustment,
                                TidyAdjustment *vadjustment)
{
  TidyListViewPrivate *priv = TIDY_LIST_VIEW (scrollable)->priv;
  
  if (hadjustment != priv->hadjustment)
    {
      if (priv->hadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->hadjustment,
                                                hadjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->hadjustment);
        }
      
      if (hadjustment)
        {
          g_object_ref (hadjustment);
          g_signal_connect (hadjustment, "notify::value",
                            G_CALLBACK (hadjustment_value_notify_cb),
                            scrollable);
        }
      
      priv->hadjustment = hadjustment;
    }

  if (vadjustment != priv->vadjustment)
    {
      if (priv->vadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->vadjustment,
                                                vadjustment_value_notify_cb,
                                                scrollable);
          g_object_unref (priv->vadjustment);
        }
      
      if (vadjustment)
        {
          g_object_ref (vadjustment);
          g_signal_connect (vadjustment, "notify::value",
                            G_CALLBACK (vadjustment_value_notify_cb),
                            scrollable);
        }
      
      priv->vadjustment = vadjustment;
    }
}

static void
tidy_list_view_refresh_hadjustment (TidyListView *view)
{
  ClutterFixed width, page_size;
  ClutterUnit clip_x, clip_width;
  
  TidyListViewPrivate *priv = view->priv;

  width = CLUTTER_UNITS_TO_FIXED (priv->allocation.x2 - priv->allocation.x1);

  clutter_actor_get_clipu (CLUTTER_ACTOR (view),
                           &clip_x, NULL,
                           &clip_width, NULL);
  if (clip_width == 0)
    page_size = width;
  else
    page_size = MIN (width, CLUTTER_UNITS_TO_FIXED (clip_width - clip_x));
  
  tidy_adjustment_set_valuesx (priv->hadjustment,
                               tidy_adjustment_get_valuex (priv->hadjustment),
                               0,
                               width,
                               CFX_ONE,
                               CFX_ONE * 20,
                               page_size);
}

static void
tidy_list_view_refresh_vadjustment (TidyListView *view)
{
  ClutterFixed height, page_size;
  ClutterUnit clip_y, clip_height;
  
  TidyListViewPrivate *priv = view->priv;

  height = CLUTTER_UNITS_TO_FIXED (priv->allocation.y2 - priv->allocation.y1);

  clutter_actor_get_clipu (CLUTTER_ACTOR (view),
                           NULL, &clip_y,
                           NULL, &clip_height);
  if (clip_height == 0)
    page_size = height;
  else
    page_size = MIN (height, CLUTTER_UNITS_TO_FIXED (clip_height - clip_y));
  
  tidy_adjustment_set_valuesx (priv->vadjustment,
                               tidy_adjustment_get_valuex (priv->vadjustment),
                               0,
                               height,
                               CFX_ONE,
                               CFX_ONE * 20,
                               page_size);
}

static void
tidy_list_view_get_adjustments (TidyScrollable *scrollable,
                                TidyAdjustment **hadjustment,
                                TidyAdjustment **vadjustment)
{
  TidyListView *view;
  TidyListViewPrivate *priv;
  
  g_return_if_fail (TIDY_IS_LIST_VIEW (scrollable));
  
  view = TIDY_LIST_VIEW (scrollable);
  priv = view->priv;
  
  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          TidyAdjustment *adjustment = tidy_adjustment_newx (0, 0, 0, 0, 0, 0);
          tidy_list_view_set_adjustments (scrollable,
                                          adjustment,
                                          priv->vadjustment);
          tidy_list_view_refresh_hadjustment (view);
          *hadjustment = adjustment;
        }
    }
  
  if (vadjustment)
    {
      if (priv->vadjustment)
        *vadjustment = priv->vadjustment;
      else
        {
          TidyAdjustment *adjustment = tidy_adjustment_newx (0, 0, 0, 0, 0, 0);
          tidy_list_view_set_adjustments (scrollable,
                                          priv->hadjustment,
                                          adjustment);
          tidy_list_view_refresh_vadjustment (view);
          *vadjustment = adjustment;
        }
    }
}

static void
tidy_scrollable_iface_init (TidyScrollableInterface *iface)
{
  iface->set_adjustments = tidy_list_view_set_adjustments;
  iface->get_adjustments = tidy_list_view_get_adjustments;
}

static void
tidy_list_view_notify_clip_cb (GObject *gobject,
                               GParamSpec *pspec,
                               gpointer user_data)
{
  TidyListView *view = TIDY_LIST_VIEW (gobject);
  
  if (view->priv->hadjustment)
    tidy_list_view_refresh_hadjustment (view);

  if (view->priv->vadjustment)
    tidy_list_view_refresh_vadjustment (view);
}

static void
tidy_list_view_init (TidyListView *view)
{
  TidyListViewPrivate *priv;

  view->priv = priv = TIDY_LIST_VIEW_GET_PRIVATE (view);

  priv->show_headers = TRUE;
  priv->rules_hint = TRUE;
  
  g_signal_connect (view, "notify::clip",
                    G_CALLBACK (tidy_list_view_notify_clip_cb),
                    NULL);
}

ClutterActor *
tidy_list_view_new (ClutterModel *model)
{
  g_return_val_if_fail (model == NULL || CLUTTER_IS_MODEL (model), NULL);

  return g_object_new (TIDY_TYPE_LIST_VIEW, "model", model, NULL);
}

void
tidy_list_view_set_model (TidyListView *view,
                          ClutterModel *model)
{
  TidyListViewPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_VIEW (view));
  g_return_if_fail (model == NULL || CLUTTER_IS_MODEL (model));
  
  priv = view->priv;

  if (priv->model)
    {
      g_signal_handler_disconnect (priv->model, priv->row_added_id);
      g_signal_handler_disconnect (priv->model, priv->row_removed_id);
      g_signal_handler_disconnect (priv->model, priv->row_changed_id);
      g_signal_handler_disconnect (priv->model, priv->sort_changed_id);
      g_signal_handler_disconnect (priv->model, priv->filter_changed_id);

      clear_layout (view, TRUE);

      g_list_foreach (priv->columns, (GFunc) g_object_unref, NULL);
      g_list_free (priv->columns);
      priv->columns = NULL;

      g_object_unref (priv->model);
      priv->model = NULL;
    }

  if (priv->hadjustment)
    tidy_adjustment_set_value (priv->hadjustment, 0.0);

  if (priv->vadjustment)
    tidy_adjustment_set_value (priv->vadjustment, 0.0);

  if (model)
    {
      TidyListViewClass *klass = TIDY_LIST_VIEW_GET_CLASS (view);
      guint n_columns, i;

      priv->model = g_object_ref (model);
      priv->n_rows = clutter_model_get_n_rows (priv->model);

      priv->columns = NULL;

      /* create a column for each column of the model */
      n_columns = clutter_model_get_n_columns (priv->model);
      for (i = 0; i < n_columns; i++)
        {
          TidyListColumn *column;

          column = klass->create_column (view, i);
          if (!column)
            continue;

          priv->columns =
            g_list_append (priv->columns, g_object_ref_sink (column));
        }

      ensure_layout (view);

      priv->row_added_id = g_signal_connect_after (priv->model, "row-added",
                                                   G_CALLBACK (on_row_added),
                                                   view);
      priv->row_removed_id = g_signal_connect_after (priv->model, "row-removed",
                                                     G_CALLBACK (on_row_removed),
                                                     view);
      priv->row_changed_id = g_signal_connect_after (priv->model, "row-changed",
                                                     G_CALLBACK (on_row_changed),
                                                     view);
      priv->sort_changed_id = g_signal_connect_after (priv->model, "sort-changed",
                                                     G_CALLBACK (on_sort_changed),
                                                     view);
      priv->filter_changed_id = g_signal_connect_after (priv->model, "filter-changed",
                                                     G_CALLBACK (on_filter_changed),
                                                     view);
    }

  g_object_notify (G_OBJECT (view), "model");

  if (CLUTTER_ACTOR_IS_VISIBLE (view))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (view));
}

ClutterModel *
tidy_list_view_get_model (TidyListView *view)
{
  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), NULL);

  return view->priv->model;
}

gint
tidy_list_view_add_column (TidyListView   *view,
                           TidyListColumn *column)
{
  TidyListViewPrivate *priv;

  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), -1);
  g_return_val_if_fail (TIDY_IS_LIST_COLUMN  (column), -1);

  priv = view->priv;

  if (g_list_find (priv->columns, column))
    {
      g_warning ("The column of type %s with title '%s' is already "
                 "bound to this list view.",
                 g_type_name (G_OBJECT_TYPE (column)),
                 tidy_list_column_get_title (column));
      return -1;
    }

  priv->columns = g_list_append (priv->columns, g_object_ref_sink (column));

  clear_layout (view, TRUE);
  ensure_layout (view);

  return g_list_length (priv->columns) - 1;
}

TidyListColumn *
tidy_list_view_get_column (TidyListView *view,
                           gint          column_id)
{
  TidyListViewPrivate *priv;

  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), NULL);

  priv = view->priv;

  return g_list_nth_data (priv->columns, column_id);
}

void
tidy_list_view_set_show_headers (TidyListView *view,
                                 gboolean      show_headers)
{
  TidyListViewPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_VIEW (view));

  priv = view->priv;

  if (priv->show_headers != show_headers)
    {
      priv->show_headers = show_headers;

      clear_layout (view, TRUE);
      ensure_layout (view);

      g_object_notify (G_OBJECT (view), "show-headers");

      if (CLUTTER_ACTOR_IS_VISIBLE (view))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (view));
    }
}

gboolean
tidy_list_view_get_show_headers (TidyListView *view)
{
  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), FALSE);

  return view->priv->show_headers;
}

void
tidy_list_view_set_rules_hint (TidyListView *view,
                               gboolean      rules_hint)
{
  TidyListViewPrivate *priv;

  g_return_if_fail (TIDY_IS_LIST_VIEW (view));

  priv = view->priv;

  if (priv->rules_hint != rules_hint)
    {
      priv->rules_hint = rules_hint;

      clear_layout (view, FALSE);
      ensure_layout (view);

      g_object_notify (G_OBJECT (view), "rules-hint");

      if (CLUTTER_ACTOR_IS_VISIBLE (view))
        clutter_actor_queue_redraw (CLUTTER_ACTOR (view));
    }
}

gboolean
tidy_list_view_get_rules_hint (TidyListView *view)
{
  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), FALSE);

  return view->priv->rules_hint;
}

gint
tidy_list_view_get_row_at_pos (TidyListView *view,
                               gint          x_coord,
                               gint          y_coord)
{
  TidyListViewPrivate *priv;
  ClutterUnit real_x, real_y;
  GList *r;
  int i;

  g_return_val_if_fail (TIDY_IS_LIST_VIEW (view), -1);

  priv = view->priv;

  if (!priv->model)
    return -1;

  if (!priv->rows)
    return -1;

  if (!clutter_actor_transform_stage_point (CLUTTER_ACTOR (view),
                                            CLUTTER_UNITS_FROM_DEVICE (x_coord),
                                            CLUTTER_UNITS_FROM_DEVICE (y_coord),
                                            &real_x, &real_y))
    return -1;

  if (priv->hadjustment)
    real_x +=
      CLUTTER_UNITS_FROM_FIXED (tidy_adjustment_get_valuex (priv->hadjustment));

  if (priv->vadjustment)
    real_y +=
      CLUTTER_UNITS_FROM_FIXED (tidy_adjustment_get_valuex (priv->vadjustment));
  
  /* If there was a sorted array of ListRow that echoed the priv->rows
   * linked list, then we could do a much faster binary search on it.
   * This implementation should be ok for a small number of rows however
   */
  for (r = priv->rows, i = 0; r; r = r->next, i++)
    {
      ListRow *row = r->data;

      if (real_y >= row->y_offset && real_y <= row->y_offset + row->height)
	return i;
    }

  return -1;
}

void
tidy_list_view_get_cell_geometry (TidyListView    *view,
                                  guint            row_index,
                                  guint            column_index,
                                  gboolean         adjust,
                                  ClutterGeometry *geometry)
{
  TidyListViewPrivate *priv;
  ClutterUnit x, y, width, height;
  ClutterUnit column_width;
  ClutterUnit h_paddingu;
  TidyListColumn *column;
  ListRow *row;
  gint i, h_padding;
  GList *l;

  g_return_if_fail (TIDY_IS_LIST_VIEW (view));

  priv = view->priv;

  if (!priv->model)
    return;

  if (!priv->rows)
    return;

  row = g_list_nth_data (priv->rows, row_index);
  if (!row)
    return;

  x = 0;
  y = row->y_offset;
  width = 0;
  height = row->height;

  tidy_stylable_get (TIDY_STYLABLE (view), "h-padding", &h_padding, NULL);
  h_paddingu = CLUTTER_UNITS_FROM_DEVICE (h_padding);

  for (i = 0, l = priv->columns; i < column_index; i++, l = l->next)
    {
      column = l->data;

      column_width = tidy_list_column_get_widthu (column);

      x += column_width;
      x += h_padding;
    }

  column = g_list_nth_data (priv->columns, column_index);
  width = tidy_list_column_get_widthu (column);

  if (adjust)
    {
      x -= CLUTTER_UNITS_FROM_FIXED (tidy_adjustment_get_valuex (priv->hadjustment));
      y -= CLUTTER_UNITS_FROM_FIXED (tidy_adjustment_get_valuex (priv->vadjustment));
    }

  geometry->x = CLUTTER_UNITS_TO_DEVICE (x);
  geometry->y = CLUTTER_UNITS_TO_DEVICE (y);
  geometry->width = CLUTTER_UNITS_TO_DEVICE (width);
  geometry->height = CLUTTER_UNITS_TO_DEVICE (height);

  return;
}
