/* tidy-viewport.h: Viewport actor
 *
 * Copyright (C) 2008 OpenedHand
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
 * Written by: Chris Lord <chris@openedhand.com>
 */

#ifndef __TIDY_VIEWPORT_H__
#define __TIDY_VIEWPORT_H__

#include <glib-object.h>
#include <clutter/clutter-group.h>

G_BEGIN_DECLS

#define TIDY_TYPE_VIEWPORT            (tidy_viewport_get_type())
#define TIDY_VIEWPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_VIEWPORT, TidyViewport))
#define TIDY_IS_VIEWPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_VIEWPORT))
#define TIDY_VIEWPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_VIEWPORT, TidyViewportClass))
#define TIDY_IS_VIEWPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_VIEWPORT))
#define TIDY_VIEWPORT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_VIEWPORT, TidyViewportClass))

typedef struct _TidyViewport          TidyViewport;
typedef struct _TidyViewportPrivate   TidyViewportPrivate;
typedef struct _TidyViewportClass     TidyViewportClass;

struct _TidyViewport
{
  ClutterGroup parent;
  
  TidyViewportPrivate *priv;
};

struct _TidyViewportClass
{
  ClutterGroupClass parent_class;
};

GType tidy_viewport_get_type (void) G_GNUC_CONST;

ClutterActor * tidy_viewport_new         (void);

void           tidy_viewport_set_originu (TidyViewport *viewport,
                                          ClutterUnit   x,
                                          ClutterUnit   y,
                                          ClutterUnit   z);

void           tidy_viewport_set_origin  (TidyViewport *viewport,
                                          gint          x,
                                          gint          y,
                                          gint          z);

void           tidy_viewport_get_originu (TidyViewport *viewport,
                                          ClutterUnit  *x,
                                          ClutterUnit  *y,
                                          ClutterUnit  *z);

void           tidy_viewport_get_origin  (TidyViewport *viewport,
                                          gint         *x,
                                          gint         *y,
                                          gint         *z);

G_END_DECLS

#endif /* __TIDY_VIEWPORT_H__ */

