/* tidy-scrollable.h: Scrollable interface
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

#ifndef __TIDY_SCROLLABLE_H__
#define __TIDY_SCROLLABLE_H__

#include <glib-object.h>
#include <tidy/tidy-adjustment.h>

G_BEGIN_DECLS

#define TIDY_TYPE_SCROLLABLE                (tidy_scrollable_get_type ())
#define TIDY_SCROLLABLE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_SCROLLABLE, TidyScrollable))
#define TIDY_IS_SCROLLABLE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_SCROLLABLE))
#define TIDY_SCROLLABLE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TIDY_TYPE_SCROLLABLE, TidyScrollableInterface))

typedef struct _TidyScrollable TidyScrollable; /* Dummy object */
typedef struct _TidyScrollableInterface TidyScrollableInterface;

struct _TidyScrollableInterface
{
  GTypeInterface parent;
  
  void (* set_adjustments) (TidyScrollable  *scrollable,
                            TidyAdjustment  *hadjustment,
                            TidyAdjustment  *vadjustment);
  void (* get_adjustments) (TidyScrollable  *scrollable,
                            TidyAdjustment **hadjustment,
                            TidyAdjustment **vadjustment);
};

GType tidy_scrollable_get_type (void) G_GNUC_CONST;

void tidy_scrollable_set_adjustments (TidyScrollable  *scrollable,
                                      TidyAdjustment  *hadjustment,
                                      TidyAdjustment  *vadjustment);
void tidy_scrollable_get_adjustments (TidyScrollable  *scrollable,
                                      TidyAdjustment **hadjustment,
                                      TidyAdjustment **vadjustment);

G_END_DECLS

#endif /* __TIDY_SCROLLABLE_H__ */

