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

#ifndef __TIDY_LIST_COLUMN_H__
#define __TIDY_LIST_COLUMN_H__

#include <glib-object.h>
#include <tidy/tidy-cell-renderer.h>

G_BEGIN_DECLS

#define TIDY_TYPE_LIST_COLUMN                   (tidy_list_column_get_type ())
#define TIDY_LIST_COLUMN(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_LIST_COLUMN, TidyListColumn))
#define TIDY_IS_LIST_COLUMN(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_LIST_COLUMN))
#define TIDY_LIST_COLUMN_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_LIST_COLUMN, TidyListColumnClass))
#define TIDY_IS_LIST_COLUMN_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_LIST_COLUMN)
#define TIDY_LIST_COLUMN_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_LIST_COLUMN, TidyListColumnClass))

typedef struct _TidyListColumn          TidyListColumn;
typedef struct _TidyListColumnPrivate   TidyListColumnPrivate;
typedef struct _TidyListColumnClass     TidyListColumnClass;

struct _TidyListColumn
{
  /*< private >*/
  GInitiallyUnowned parent_instance;

  TidyListColumnPrivate *priv;
};

struct _TidyListColumnClass
{
  /*< private >*/
  GInitiallyUnownedClass parent_class;
};

GType                 tidy_list_column_get_type            (void) G_GNUC_CONST;

void                  tidy_list_column_set_header_renderer (TidyListColumn   *column,
                                                            TidyCellRenderer *renderer);
TidyCellRenderer *    tidy_list_column_get_header_renderer (TidyListColumn   *column);
void                  tidy_list_column_set_cell_renderer   (TidyListColumn   *column,
                                                            TidyCellRenderer *renderer);
TidyCellRenderer *    tidy_list_column_get_cell_renderer   (TidyListColumn   *column);
void                  tidy_list_column_set_max_width       (TidyListColumn   *column,
                                                            gint              max_width);
void                  tidy_list_column_set_max_widthu      (TidyListColumn   *column,
                                                            ClutterUnit       max_width);
gint                  tidy_list_column_get_max_width       (TidyListColumn   *column);
ClutterUnit           tidy_list_column_get_max_widthu      (TidyListColumn   *column);
void                  tidy_list_column_set_min_width       (TidyListColumn   *column,
                                                            gint              min_width);
void                  tidy_list_column_set_min_widthu      (TidyListColumn   *column,
                                                            ClutterUnit       min_width);
gint                  tidy_list_column_get_min_width       (TidyListColumn   *column);
ClutterUnit           tidy_list_column_get_min_widthu      (TidyListColumn   *column);
void                  tidy_list_column_set_visible         (TidyListColumn   *column,
                                                            gboolean          visible);
gboolean              tidy_list_column_get_visible         (TidyListColumn   *column);

guint                 tidy_list_column_get_model_index     (TidyListColumn   *column);
G_CONST_RETURN gchar *tidy_list_column_get_title           (TidyListColumn   *column);
guint                 tidy_list_column_get_width           (TidyListColumn   *column);
ClutterUnit           tidy_list_column_get_widthu          (TidyListColumn   *column);

G_END_DECLS

#endif /* __TIDY_LIST_COLUMN_H__ */
