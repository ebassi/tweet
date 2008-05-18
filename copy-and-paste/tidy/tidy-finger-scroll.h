/* tidy-finger-scroll.h: Finger scrolling container actor
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

#ifndef __TIDY_FINGER_SCROLL_H__
#define __TIDY_FINGER_SCROLL_H__

#include <glib-object.h>
#include <tidy/tidy-scroll-view.h>

G_BEGIN_DECLS

#define TIDY_TYPE_FINGER_SCROLL            (tidy_finger_scroll_get_type())
#define TIDY_FINGER_SCROLL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_FINGER_SCROLL, TidyFingerScroll))
#define TIDY_IS_FINGER_SCROLL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_FINGER_SCROLL))
#define TIDY_FINGER_SCROLL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_FINGER_SCROLL, TidyFingerScrollClass))
#define TIDY_IS_FINGER_SCROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_FINGER_SCROLL))
#define TIDY_FINGER_SCROLL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_FINGER_SCROLL, TidyFingerScrollClass))

/**
 * TidyFingerScrollMode:
 * @TIDY_FINGER_SCROLL_MODE_PUSH: Non-kinetic scrolling
 * @TIDY_FINGER_SCROLL_MODE_KINETIC: Kinetic scrolling
 *
 * Type of scrolling.
 */
typedef enum {
  TIDY_FINGER_SCROLL_MODE_PUSH,
  TIDY_FINGER_SCROLL_MODE_KINETIC
} TidyFingerScrollMode;

typedef struct _TidyFingerScroll          TidyFingerScroll;
typedef struct _TidyFingerScrollPrivate   TidyFingerScrollPrivate;
typedef struct _TidyFingerScrollClass     TidyFingerScrollClass;

struct _TidyFingerScroll
{
  /*< private >*/
  TidyScrollView parent_instance;
  
  TidyFingerScrollPrivate *priv;
};

struct _TidyFingerScrollClass
{
  TidyScrollViewClass parent_class;
};

GType tidy_finger_scroll_get_type (void) G_GNUC_CONST;

ClutterActor *tidy_finger_scroll_new  (TidyFingerScrollMode mode);

void          tidy_finger_scroll_stop (TidyFingerScroll *scroll);

G_END_DECLS

#endif /* __TIDY_FINGER_SCROLL_H__ */
