/* tidy-scroll-bar.h: Scroll bar actor
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

#ifndef __TIDY_SCROLL_BAR_H__
#define __TIDY_SCROLL_BAR_H__

#include <tidy/tidy-actor.h>
#include <tidy/tidy-adjustment.h>

G_BEGIN_DECLS

#define TIDY_TYPE_SCROLL_BAR            (tidy_scroll_bar_get_type())
#define TIDY_SCROLL_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_SCROLL_BAR, TidyScrollBar))
#define TIDY_IS_SCROLL_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_SCROLL_BAR))
#define TIDY_SCROLL_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_SCROLL_BAR, TidyScrollBarClass))
#define TIDY_IS_SCROLL_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_SCROLL_BAR))
#define TIDY_SCROLL_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_SCROLL_BAR, TidyScrollBarClass))

typedef struct _TidyScrollBar          TidyScrollBar;
typedef struct _TidyScrollBarPrivate   TidyScrollBarPrivate;
typedef struct _TidyScrollBarClass     TidyScrollBarClass;

struct _TidyScrollBar
{
  /*< private >*/
  TidyActor parent_instance;
  
  TidyScrollBarPrivate *priv;
};

struct _TidyScrollBarClass
{
  TidyActorClass parent_class;
};

GType tidy_scroll_bar_get_type (void) G_GNUC_CONST;

ClutterActor *  tidy_scroll_bar_new            (TidyAdjustment *adjustment);
ClutterActor *  tidy_scroll_bar_new_with_handle(TidyAdjustment *adjustment,
                                                ClutterActor   *handle);
void            tidy_scroll_bar_set_adjustment (TidyScrollBar  *bar,
                                                TidyAdjustment *adjustment);
TidyAdjustment *tidy_scroll_bar_get_adjustment (TidyScrollBar  *bar);
void            tidy_scroll_bar_set_handle     (TidyScrollBar  *bar, 
                                                ClutterActor   *handle);
ClutterActor *  tidy_scroll_bar_get_handle     (TidyScrollBar  *bar);
void            tidy_scroll_bar_set_texture    (TidyScrollBar  *bar,
                                                ClutterActor   *texture);
ClutterActor *  tidy_scroll_bar_get_texture    (TidyScrollBar  *bar);

G_END_DECLS

#endif /* __TIDY_SCROLL_BAR_H__ */

