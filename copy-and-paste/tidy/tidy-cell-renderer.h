/* tidy-cell-renderer.h: Base class for cell renderers
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

#ifndef __TIDY_CELL_RENDERER_H__
#define __TIDY_CELL_RENDERER_H__

#include <glib-object.h>
#include <clutter/clutter-actor.h>
#include <tidy/tidy-actor.h>

G_BEGIN_DECLS

#define TIDY_TYPE_CELL_RENDERER                 (tidy_cell_renderer_get_type ())
#define TIDY_CELL_RENDERER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_CELL_RENDERER, TidyCellRenderer))
#define TIDY_IS_CELL_RENDERER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_CELL_RENDERER))
#define TIDY_CELL_RENDERER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_CELL_RENDERER, TidyCellRendererClass))
#define TIDY_IS_CELL_RENDERER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_CELL_RENDERER))
#define TIDY_CELL_RENDERER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_CELL_RENDERER, TidyCellRendererClass))

typedef enum {
  TIDY_CELL_HEADER,
  TIDY_CELL_NORMAL,
  TIDY_CELL_SELECTED,
  TIDY_CELL_INACTIVE,
  TIDY_CELL_SORTING
} TidyCellState;

typedef struct _TidyCellRenderer                TidyCellRenderer;
typedef struct _TidyCellRendererPrivate         TidyCellRendererPrivate;
typedef struct _TidyCellRendererClass           TidyCellRendererClass;

struct _TidyCellRenderer
{
  /*< private >*/
  GInitiallyUnowned parent_instance;

  TidyCellRendererPrivate *priv;
};

struct _TidyCellRendererClass
{
  /*< private >*/
  GInitiallyUnownedClass parent_class;

  /*< public >*/
  ClutterActor *(* get_cell_actor) (TidyCellRenderer *renderer,
                                    TidyActor        *list_view,
                                    const GValue     *value,
                                    TidyCellState     state,
                                    ClutterGeometry  *size,
                                    gint              row,
                                    gint              column);
};

GType         tidy_cell_renderer_get_type       (void) G_GNUC_CONST;
ClutterActor *tidy_cell_renderer_get_cell_actor (TidyCellRenderer *renderer,
                                                 TidyActor        *list_view,
                                                 const GValue     *value,
                                                 TidyCellState     state,
                                                 ClutterGeometry  *size,
                                                 gint              row,
                                                 gint              column);
void          tidy_cell_renderer_get_alignment  (TidyCellRenderer *renderer,
                                                 gdouble          *x_align,
                                                 gdouble          *y_align);
void          tidy_cell_renderer_get_alignmentx (TidyCellRenderer *renderer,
                                                 ClutterFixed     *x_align,
                                                 ClutterFixed     *y_align);
void          tidy_cell_renderer_set_alignment  (TidyCellRenderer *renderer,
                                                 gdouble           x_align,
                                                 gdouble           y_align);
void          tidy_cell_renderer_set_alignmentx (TidyCellRenderer *renderer,
                                                 ClutterFixed      x_align,
                                                 ClutterFixed      y_align);


G_END_DECLS

#endif /* __TIDY_CELL_RENDERER_H__ */
