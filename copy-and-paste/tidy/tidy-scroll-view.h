/* tidy-scroll-view.h: Container with scroll-bars
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

#ifndef __TIDY_SCROLL_VIEW_H__
#define __TIDY_SCROLL_VIEW_H__

#include <glib-object.h>
#include <tidy/tidy-actor.h>

G_BEGIN_DECLS

#define TIDY_TYPE_SCROLL_VIEW            (tidy_scroll_view_get_type())
#define TIDY_SCROLL_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_SCROLL_VIEW, TidyScrollView))
#define TIDY_IS_SCROLL_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_SCROLL_VIEW))
#define TIDY_SCROLL_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_SCROLL_VIEW, TidyScrollViewClass))
#define TIDY_IS_SCROLL_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_SCROLL_VIEW))
#define TIDY_SCROLL_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_SCROLL_VIEW, TidyScrollViewClass))

typedef struct _TidyScrollView          TidyScrollView;
typedef struct _TidyScrollViewPrivate   TidyScrollViewPrivate;
typedef struct _TidyScrollViewClass     TidyScrollViewClass;

struct _TidyScrollView
{
  /*< private >*/
  TidyActor parent;
  
  TidyScrollViewPrivate *priv;
};

struct _TidyScrollViewClass
{
  TidyActorClass parent_class;
};

GType tidy_scroll_view_get_type (void) G_GNUC_CONST;

ClutterActor *tidy_scroll_view_new             (void);

ClutterActor *tidy_scroll_view_get_hscroll_bar (TidyScrollView *scroll);
ClutterActor *tidy_scroll_view_get_vscroll_bar (TidyScrollView *scroll);
ClutterActor *tidy_scroll_view_get_child       (TidyScrollView *scroll);

G_END_DECLS

#endif /* __TIDY_SCROLL_VIEW_H__ */
