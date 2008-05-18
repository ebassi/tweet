/* tidy-adjustment.c: Adjustment object
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>
#include <clutter/clutter-fixed.h>
#include <clutter/clutter-timeline.h>

#include "tidy-adjustment.h"
#include "tidy-marshal.h"
#include "tidy-private.h"

G_DEFINE_TYPE (TidyAdjustment, tidy_adjustment, G_TYPE_OBJECT)

#define ADJUSTMENT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TIDY_TYPE_ADJUSTMENT, TidyAdjustmentPrivate))

struct _TidyAdjustmentPrivate
{
  ClutterFixed lower;
  ClutterFixed upper;
  ClutterFixed value;
  ClutterFixed step_increment;
  ClutterFixed page_increment;
  ClutterFixed page_size;

  /* For interpolation */
  ClutterTimeline *interpolation;
  ClutterFixed     dx;
  ClutterFixed     old_position;
  ClutterFixed     new_position;
};

enum
{
  PROP_0,

  PROP_LOWER,
  PROP_UPPER,
  PROP_VALUE,
  PROP_STEP_INC,
  PROP_PAGE_INC,
  PROP_PAGE_SIZE,
};

enum
{
  CHANGED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void tidy_adjustment_set_lower          (TidyAdjustment *adjustment,
                                                gdouble         lower);
static void tidy_adjustment_set_upper          (TidyAdjustment *adjustment,
                                                gdouble         upper);
static void tidy_adjustment_set_step_increment (TidyAdjustment *adjustment,
                                                gdouble         step);
static void tidy_adjustment_set_page_increment (TidyAdjustment *adjustment,
                                                gdouble         page);
static void tidy_adjustment_set_page_size      (TidyAdjustment *adjustment,
                                                gdouble         size);

static void
tidy_adjustment_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  TidyAdjustmentPrivate *priv = TIDY_ADJUSTMENT (object)->priv;

  switch (prop_id)
    {
    case PROP_LOWER:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->lower));
      break;

    case PROP_UPPER:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->upper));
      break;

    case PROP_VALUE:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->value));
      break;

    case PROP_STEP_INC:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->step_increment));
      break;

    case PROP_PAGE_INC:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->page_increment));
      break;

    case PROP_PAGE_SIZE:
      g_value_set_double (value, CLUTTER_FIXED_TO_DOUBLE (priv->page_size));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
tidy_adjustment_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  TidyAdjustment *adj = TIDY_ADJUSTMENT (object);

  switch (prop_id)
    {
    case PROP_LOWER:
      tidy_adjustment_set_lower (adj, g_value_get_double (value));
      break;

    case PROP_UPPER:
      tidy_adjustment_set_upper (adj, g_value_get_double (value));
      break;

    case PROP_VALUE:
      tidy_adjustment_set_value (adj, g_value_get_double (value));
      break;

    case PROP_STEP_INC:
      tidy_adjustment_set_step_increment (adj, g_value_get_double (value));
      break;

    case PROP_PAGE_INC:
      tidy_adjustment_set_page_increment (adj, g_value_get_double (value));
      break;

    case PROP_PAGE_SIZE:
      tidy_adjustment_set_page_size (adj, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
stop_interpolation (TidyAdjustment *adjustment)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;

  if (priv->interpolation)
    {
      clutter_timeline_stop (priv->interpolation);
      g_object_unref (priv->interpolation);
      priv->interpolation = NULL;
    }
}

static void
tidy_adjustment_dispose (GObject *object)
{
  stop_interpolation (TIDY_ADJUSTMENT (object));
  
  G_OBJECT_CLASS (tidy_adjustment_parent_class)->dispose (object);
}

static void
tidy_adjustment_class_init (TidyAdjustmentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyAdjustmentPrivate));

  object_class->get_property = tidy_adjustment_get_property;
  object_class->set_property = tidy_adjustment_set_property;
  object_class->dispose = tidy_adjustment_dispose;
  
  g_object_class_install_property (object_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
                                                        "Lower",
                                                        "Lower bound",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
                                                        "Upper",
                                                        "Upper bound",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_VALUE,
                                   g_param_spec_double ("value",
                                                        "Value",
                                                        "Current value",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_STEP_INC,
                                   g_param_spec_double ("step-increment",
                                                        "Step Increment",
                                                        "Step increment",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_PAGE_INC,
                                   g_param_spec_double ("page-increment",
                                                        "Page Increment",
                                                        "Page increment",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_PAGE_SIZE,
                                   g_param_spec_double ("page-size",
                                                        "Page Size",
                                                        "Page size",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        TIDY_PARAM_READWRITE));

  signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TidyAdjustmentClass, changed),
                  NULL, NULL,
                  _tidy_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
tidy_adjustment_init (TidyAdjustment *self)
{
  self->priv = ADJUSTMENT_PRIVATE (self);
}

TidyAdjustment *
tidy_adjustment_new (gdouble value,
                     gdouble lower,
                     gdouble upper,
                     gdouble step_increment,
                     gdouble page_increment,
                     gdouble page_size)
{
  return g_object_new (TIDY_TYPE_ADJUSTMENT,
                       "value", value,
                       "lower", lower,
                       "upper", upper,
                       "step-increment", step_increment,
                       "page-increment", page_increment,
                       "page-size", page_size,
                       NULL);
}

TidyAdjustment *
tidy_adjustment_newx (ClutterFixed value,
                      ClutterFixed lower,
                      ClutterFixed upper,
                      ClutterFixed step_increment,
                      ClutterFixed page_increment,
                      ClutterFixed page_size)
{
  TidyAdjustment *retval;
  TidyAdjustmentPrivate *priv;

  retval = g_object_new (TIDY_TYPE_ADJUSTMENT, NULL);
  priv = retval->priv;

  priv->value = value;
  priv->lower = lower;
  priv->upper = upper;
  priv->step_increment = step_increment;
  priv->page_increment = page_increment;
  priv->page_size = page_size;

  return retval;
}

ClutterFixed
tidy_adjustment_get_valuex (TidyAdjustment *adjustment)
{
  TidyAdjustmentPrivate *priv;
  
  g_return_val_if_fail (TIDY_IS_ADJUSTMENT (adjustment), 0);
  
  priv = adjustment->priv;
  
  if (adjustment->priv->interpolation)
    {
      return MAX (priv->lower, MIN (priv->upper - priv->page_size,
                                    adjustment->priv->new_position));
    }
  else
    return adjustment->priv->value;
}

gdouble
tidy_adjustment_get_value (TidyAdjustment *adjustment)
{
  g_return_val_if_fail (TIDY_IS_ADJUSTMENT (adjustment), 0.0);

  return CLUTTER_FIXED_TO_FLOAT (adjustment->priv->value);
}

void
tidy_adjustment_set_valuex (TidyAdjustment *adjustment,
                            ClutterFixed    value)
{
  TidyAdjustmentPrivate *priv;
  
  g_return_if_fail (TIDY_IS_ADJUSTMENT (adjustment));
  
  priv = adjustment->priv;

  stop_interpolation (adjustment);

  value = CLAMP (value, priv->lower, MAX (priv->lower,
                                          priv->upper - priv->page_size));

  if (priv->value != value)
    {
      priv->value = value;
      g_object_notify (G_OBJECT (adjustment), "value");
    }
}

void
tidy_adjustment_set_value (TidyAdjustment *adjustment,
                           gdouble         value)
{
  tidy_adjustment_set_valuex (adjustment, CLUTTER_FLOAT_TO_FIXED (value));
}

void
tidy_adjustment_clamp_pagex (TidyAdjustment *adjustment,
                             ClutterFixed    lower,
                             ClutterFixed    upper)
{
  gboolean changed;
  TidyAdjustmentPrivate *priv;
  
  g_return_if_fail (TIDY_IS_ADJUSTMENT (adjustment));
  
  priv = adjustment->priv;
  
  stop_interpolation (adjustment);

  lower = CLAMP (lower, priv->lower, priv->upper - priv->page_size);
  upper = CLAMP (upper, priv->lower + priv->page_size, priv->upper);
  
  changed = FALSE;
  
  if (priv->value + priv->page_size > upper)
    {
      priv->value = upper - priv->page_size;
      changed = TRUE;
    }

  if (priv->value < lower)
    {
      priv->value = lower;
      changed = TRUE;
    }
  
  if (changed)
    g_object_notify (G_OBJECT (adjustment), "value");
}

void
tidy_adjustment_clamp_page (TidyAdjustment *adjustment,
                            gdouble         lower,
                            gdouble         upper)
{
  tidy_adjustment_clamp_pagex (adjustment,
                               CLUTTER_FLOAT_TO_FIXED (lower),
                               CLUTTER_FLOAT_TO_FIXED (upper));
}

static void
tidy_adjustment_set_lower (TidyAdjustment *adjustment,
                           gdouble         lower)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  ClutterFixed value = CLUTTER_FLOAT_TO_FIXED (lower);
  
  if (priv->lower != value)
    {
      priv->lower = value;

      g_signal_emit (adjustment, signals[CHANGED], 0);

      g_object_notify (G_OBJECT (adjustment), "lower");

      tidy_adjustment_clamp_pagex (adjustment, priv->lower, priv->upper);
    }
}

static void
tidy_adjustment_set_upper (TidyAdjustment *adjustment,
                           gdouble         upper)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  ClutterFixed value = CLUTTER_FLOAT_TO_FIXED (upper);
  
  if (priv->upper != value)
    {
      priv->upper = value;

      g_signal_emit (adjustment, signals[CHANGED], 0);

      g_object_notify (G_OBJECT (adjustment), "upper");
      
      tidy_adjustment_clamp_pagex (adjustment, priv->lower, priv->upper);
    }
}

static void
tidy_adjustment_set_step_increment (TidyAdjustment *adjustment,
                                    gdouble         step)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  ClutterFixed value = CLUTTER_FLOAT_TO_FIXED (step);
  
  if (priv->step_increment != value)
    {
      priv->step_increment = value;

      g_signal_emit (adjustment, signals[CHANGED], 0);

      g_object_notify (G_OBJECT (adjustment), "step-increment");
    }
}

static void
tidy_adjustment_set_page_increment (TidyAdjustment *adjustment,
                                    gdouble        page)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  ClutterFixed value = CLUTTER_FLOAT_TO_FIXED (page);

  if (priv->page_increment != value)
    {
      priv->page_increment = value;

      g_signal_emit (adjustment, signals[CHANGED], 0);

      g_object_notify (G_OBJECT (adjustment), "page-increment");
    }
}

static void
tidy_adjustment_set_page_size (TidyAdjustment *adjustment,
                               gdouble         size)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  ClutterFixed value = CLUTTER_FLOAT_TO_FIXED (size);

  if (priv->page_size != value)
    {
      priv->page_size = value;

      g_signal_emit (adjustment, signals[CHANGED], 0);

      g_object_notify (G_OBJECT (adjustment), "page_size");

      tidy_adjustment_clamp_pagex (adjustment, priv->lower, priv->upper);
    }
}

void
tidy_adjustment_set_valuesx (TidyAdjustment *adjustment,
                             ClutterFixed    value,
                             ClutterFixed    lower,
                             ClutterFixed    upper,
                             ClutterFixed    step_increment,
                             ClutterFixed    page_increment,
                             ClutterFixed    page_size)
{
  TidyAdjustmentPrivate *priv;
  gboolean emit_changed = FALSE;
  
  g_return_if_fail (TIDY_IS_ADJUSTMENT (adjustment));
  
  priv = adjustment->priv;
  
  stop_interpolation (adjustment);

  emit_changed = FALSE;
  
  g_object_freeze_notify (G_OBJECT (adjustment));

  if (priv->lower != lower)
    {
      priv->lower = lower;
      emit_changed = TRUE;

      g_object_notify (G_OBJECT (adjustment), "lower");
    }

  if (priv->upper != upper)
    {
      priv->upper = upper;
      emit_changed = TRUE;

      g_object_notify (G_OBJECT (adjustment), "upper");
    }

  if (priv->step_increment != step_increment)
    {
      priv->step_increment = step_increment;
      emit_changed = TRUE;

      g_object_notify (G_OBJECT (adjustment), "step-increment");
    }

  if (priv->page_increment != page_increment)
    {
      priv->page_increment = page_increment;
      emit_changed = TRUE;

      g_object_notify (G_OBJECT (adjustment), "page-increment");
    }

  if (priv->page_size != page_size)
    {
      priv->page_size = page_size;
      emit_changed = TRUE;

      g_object_notify (G_OBJECT (adjustment), "page-size");
    }
  
  tidy_adjustment_set_valuex (adjustment, value);

  if (emit_changed)
    g_signal_emit (G_OBJECT (adjustment), signals[CHANGED], 0);

  g_object_thaw_notify (G_OBJECT (adjustment));
}

void
tidy_adjustment_set_values (TidyAdjustment *adjustment,
                            gdouble         value,
                            gdouble         lower,
                            gdouble         upper,
                            gdouble         step_increment,
                            gdouble         page_increment,
                            gdouble         page_size)
{
  tidy_adjustment_set_valuesx (adjustment,
                               CLUTTER_FLOAT_TO_FIXED (value),
                               CLUTTER_FLOAT_TO_FIXED (lower),
                               CLUTTER_FLOAT_TO_FIXED (upper),
                               CLUTTER_FLOAT_TO_FIXED (step_increment),
                               CLUTTER_FLOAT_TO_FIXED (page_increment),
                               CLUTTER_FLOAT_TO_FIXED (page_size));
}

void
tidy_adjustment_get_valuesx (TidyAdjustment *adjustment,
                             ClutterFixed   *value,
                             ClutterFixed   *lower,
                             ClutterFixed   *upper,
                             ClutterFixed   *step_increment,
                             ClutterFixed   *page_increment,
                             ClutterFixed   *page_size)
{
  TidyAdjustmentPrivate *priv;
  
  g_return_if_fail (TIDY_IS_ADJUSTMENT (adjustment));
  
  priv = adjustment->priv;
  
  if (lower)
    *lower = priv->lower;

  if (upper)
    *upper = priv->upper;

  if (value)
    *value = tidy_adjustment_get_valuex (adjustment);

  if (step_increment)
    *step_increment = priv->step_increment;

  if (page_increment)
    *page_increment = priv->page_increment;

  if (page_size)
    *page_size = priv->page_size;
}

void
tidy_adjustment_get_values (TidyAdjustment *adjustment,
                            gdouble        *value,
                            gdouble        *lower,
                            gdouble        *upper,
                            gdouble        *step_increment,
                            gdouble        *page_increment,
                            gdouble        *page_size)
{
  TidyAdjustmentPrivate *priv;
  
  g_return_if_fail (TIDY_IS_ADJUSTMENT (adjustment));
  
  priv = adjustment->priv;
  
  if (lower)
    *lower = CLUTTER_FIXED_TO_DOUBLE (priv->lower);

  if (upper)
    *upper = CLUTTER_FIXED_TO_DOUBLE (priv->upper);

  if (value)
    *value = CLUTTER_FIXED_TO_DOUBLE (tidy_adjustment_get_valuex (adjustment));

  if (step_increment)
    *step_increment = CLUTTER_FIXED_TO_DOUBLE (priv->step_increment);

  if (page_increment)
    *page_increment = CLUTTER_FIXED_TO_DOUBLE (priv->page_increment);

  if (page_size)
    *page_size = CLUTTER_FIXED_TO_DOUBLE (priv->page_size);
}

static void
interpolation_new_frame_cb (ClutterTimeline *timeline,
                            gint             frame_num,
                            TidyAdjustment  *adjustment)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;

  priv->interpolation = NULL;
  tidy_adjustment_set_valuex (adjustment,
                              priv->old_position +
                              clutter_qmulx (CLUTTER_INT_TO_FIXED (frame_num),
                                             priv->dx));
  priv->interpolation = timeline;
}

static void
interpolation_completed_cb (ClutterTimeline *timeline,
                            TidyAdjustment  *adjustment)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;
  
  g_object_unref (timeline);
  priv->interpolation = NULL;

  tidy_adjustment_set_valuex (adjustment,
                              priv->new_position);
}

void
tidy_adjustment_interpolatex (TidyAdjustment *adjustment,
                              ClutterFixed    value,
                              guint           n_frames,
                              guint           fps)
{
  TidyAdjustmentPrivate *priv = adjustment->priv;

  stop_interpolation (adjustment);
  
  if (n_frames <= 1)
    {
      tidy_adjustment_set_valuex (adjustment, value);
      return;
    }

  priv->old_position = priv->value;
  priv->new_position = value;
  
  priv->dx = clutter_qdivx (priv->new_position - priv->old_position,
                            CLUTTER_INT_TO_FIXED (n_frames));
  priv->interpolation = clutter_timeline_new (n_frames, fps);
  
  g_signal_connect (priv->interpolation,
                    "new-frame",
                    G_CALLBACK (interpolation_new_frame_cb),
                    adjustment);
  g_signal_connect (priv->interpolation,
                    "completed",
                    G_CALLBACK (interpolation_completed_cb),
                    adjustment);
  
  clutter_timeline_start (priv->interpolation);
}

void
tidy_adjustment_interpolate (TidyAdjustment *adjustment,
                              gdouble        value,
                              guint          n_frames,
                              guint          fps)
{
  tidy_adjustment_interpolatex (adjustment,
                                CLUTTER_FLOAT_TO_FIXED (value),
                                n_frames,
                                fps);
}
