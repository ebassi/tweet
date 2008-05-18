/* tidy-adjustment.h: Adjustment object
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
 * Written by: Chris Lord <chris@openedhand.com>, inspired by GtkAdjustment
 */

#ifndef __TIDY_ADJUSTMENT_H__
#define __TIDY_ADJUSTMENT_H__

#include <glib-object.h>
#include <clutter/clutter-fixed.h>

G_BEGIN_DECLS

#define TIDY_TYPE_ADJUSTMENT            (tidy_adjustment_get_type())
#define TIDY_ADJUSTMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_ADJUSTMENT, TidyAdjustment))
#define TIDY_IS_ADJUSTMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_ADJUSTMENT))
#define TIDY_ADJUSTMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_ADJUSTMENT, TidyAdjustmentClass))
#define TIDY_IS_ADJUSTMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_ADJUSTMENT))
#define TIDY_ADJUSTMENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_ADJUSTMENT, TidyAdjustmentClass))

typedef struct _TidyAdjustment          TidyAdjustment;
typedef struct _TidyAdjustmentPrivate   TidyAdjustmentPrivate;
typedef struct _TidyAdjustmentClass     TidyAdjustmentClass;

/**
 * TidyAdjustment:
 *
 * Class for handling an interval between to values. The contents of
 * the #TidyAdjustment are private and should be accessed using the
 * public API.
 */
struct _TidyAdjustment
{
  /*< private >*/
  GObject parent_instance;
  
  TidyAdjustmentPrivate *priv;
};

/**
 * TidyAdjustmentClass
 * @changed: Class handler for the ::changed signal.
 *
 * Base class for #TidyAdjustment.
 */
struct _TidyAdjustmentClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  void (* changed) (TidyAdjustment *adjustment);
};

GType tidy_adjustment_get_type (void) G_GNUC_CONST;

TidyAdjustment *tidy_adjustment_new          (gdouble         value,
                                              gdouble         lower,
                                              gdouble         upper,
                                              gdouble         step_increment,
                                              gdouble         page_increment,
                                              gdouble         page_size);
TidyAdjustment *tidy_adjustment_newx         (ClutterFixed    value,
                                              ClutterFixed    lower,
                                              ClutterFixed    upper,
                                              ClutterFixed    step_increment,
                                              ClutterFixed    page_increment,
                                              ClutterFixed    page_size);
gdouble         tidy_adjustment_get_value    (TidyAdjustment *adjustment);
ClutterFixed    tidy_adjustment_get_valuex   (TidyAdjustment *adjustment);
void            tidy_adjustment_set_value    (TidyAdjustment *adjustment,
                                              gdouble         value);
void            tidy_adjustment_set_valuex   (TidyAdjustment *adjustment,
                                              ClutterFixed    value);
void            tidy_adjustment_clamp_page   (TidyAdjustment *adjustment,
                                              gdouble         lower,
                                              gdouble         upper);
void            tidy_adjustment_clamp_pagex  (TidyAdjustment *adjustment,
                                              ClutterFixed    lower,
                                              ClutterFixed    upper);
void            tidy_adjustment_set_values   (TidyAdjustment *adjustment,
                                              gdouble         value,
                                              gdouble         lower,
                                              gdouble         upper,
                                              gdouble         step_increment,
                                              gdouble         page_increment,
                                              gdouble         page_size);
void            tidy_adjustment_set_valuesx  (TidyAdjustment *adjustment,
                                              ClutterFixed    value,
                                              ClutterFixed    lower,
                                              ClutterFixed    upper,
                                              ClutterFixed    step_increment,
                                              ClutterFixed    page_increment,
                                              ClutterFixed    page_size);
void            tidy_adjustment_get_values   (TidyAdjustment *adjustment,
                                              gdouble        *value,
                                              gdouble        *lower,
                                              gdouble        *upper,
                                              gdouble        *step_increment,
                                              gdouble        *page_increment,
                                              gdouble        *page_size);
void            tidy_adjustment_get_valuesx  (TidyAdjustment *adjustment,
                                              ClutterFixed   *value,
                                              ClutterFixed   *lower,
                                              ClutterFixed   *upper,
                                              ClutterFixed   *step_increment,
                                              ClutterFixed   *page_increment,
                                              ClutterFixed   *page_size);

void            tidy_adjustment_interpolate  (TidyAdjustment *adjustment,
                                              gdouble         value,
                                              guint           n_frames,
                                              guint           fps);
void            tidy_adjustment_interpolatex (TidyAdjustment *adjustment,
                                              ClutterFixed    value,
                                              guint           n_frames,
                                              guint           fps);

G_END_DECLS

#endif /* __TIDY_ADJUSTMENT_H__ */

