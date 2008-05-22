/* tidy-list-view.h: List actor
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

#ifndef __TIDY_LIST_VIEW_H__
#define __TIDY_LIST_VIEW_H__

#include <clutter/clutter-actor.h>
#include <clutter/clutter-model.h>

#include <tidy/tidy-actor.h>
#include <tidy/tidy-list-column.h>

G_BEGIN_DECLS

#define TIDY_TYPE_LIST_VIEW             (tidy_list_view_get_type ())
#define TIDY_LIST_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_LIST_VIEW, TidyListView))
#define TIDY_IS_LIST_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_LIST_VIEW))
#define TIDY_LIST_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_LIST_VIEW, TidyListViewClass))
#define TIDY_IS_LIST_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_LIST_VIEW))
#define TIDY_LIST_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_LIST_VIEW, TidyListViewClass))

typedef struct _TidyListView            TidyListView;
typedef struct _TidyListViewPrivate     TidyListViewPrivate;
typedef struct _TidyListViewClass       TidyListViewClass;

struct _TidyListView
{
  TidyActor parent_instance;

  TidyListViewPrivate *priv;
};

struct _TidyListViewClass
{
  TidyActorClass parent_class;

  /* vfuncs, not signals */
  TidyListColumn *(* create_column) (TidyListView *list_view,
                                     guint         model_id);

  /* signals */
  void (* row_clicked) (TidyListView     *list_view,
                        ClutterModel     *model,
                        ClutterModelIter *iter);
};

GType tidy_list_view_get_type (void) G_GNUC_CONST;

ClutterActor *  tidy_list_view_new               (ClutterModel    *model);

void            tidy_list_view_set_model         (TidyListView    *view,
                                                  ClutterModel    *model);
ClutterModel *  tidy_list_view_get_model         (TidyListView    *view);

gint            tidy_list_view_add_column        (TidyListView    *view,
                                                  TidyListColumn  *column);
TidyListColumn *tidy_list_view_get_column        (TidyListView    *view,
                                                  gint             column_id);

void            tidy_list_view_set_show_headers  (TidyListView    *view,
                                                  gboolean         show_headers);
gboolean        tidy_list_view_get_show_headers  (TidyListView    *view);
void            tidy_list_view_set_rules_hint    (TidyListView    *view,
                                                  gboolean         rules_hint);
gboolean        tidy_list_view_get_rules_hint    (TidyListView    *view);

gint            tidy_list_view_get_row_at_pos    (TidyListView    *view,
                                                  gint             x_coord,
                                                  gint             y_coord);
void            tidy_list_view_get_cell_geometry (TidyListView    *view,
                                                  guint            row_index,
                                                  guint            column_index,
                                                  gboolean         adjust,
                                                  ClutterGeometry *geometry);

G_END_DECLS

#endif /* __TIDY_LIST_VIEW_H__ */
