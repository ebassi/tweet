/* tidy-finger-scroll.c: Finger scrolling container actor
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

#include "tidy-finger-scroll.h"
#include "tidy-enum-types.h"
#include "tidy-marshal.h"
#include "tidy-scroll-bar.h"
#include "tidy-scrollable.h"
#include "tidy-scroll-view.h"
#include <clutter/clutter.h>
#include <math.h>

G_DEFINE_TYPE (TidyFingerScroll, tidy_finger_scroll, TIDY_TYPE_SCROLL_VIEW)

#define FINGER_SCROLL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  TIDY_TYPE_FINGER_SCROLL, \
                                  TidyFingerScrollPrivate))

struct _TidyFingerScrollPrivate
{
  /* Scroll mode */
  TidyFingerScrollMode mode;
  
  /* Units to store the origin of a click when scrolling */
  ClutterUnit            x_origin;
  ClutterUnit            y_origin;
  
  /* Variables for storing acceleration information for kinetic mode */
  GTimeVal               last_motion_time;
  ClutterTimeline       *deceleration_timeline;
  ClutterUnit            dx;
  ClutterUnit            dy;
  ClutterFixed           decel_rate;
  
  /* Variables to fade in/out scroll-bars */
  ClutterEffectTemplate *template;
  ClutterTimeline       *hscroll_timeline;
  ClutterTimeline       *vscroll_timeline;
};

enum {
  PROP_MODE = 1,
};

static void
tidy_finger_scroll_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  TidyFingerScrollPrivate *priv = TIDY_FINGER_SCROLL (object)->priv;
  
  switch (property_id)
    {
    case PROP_MODE :
      g_value_set_enum (value, priv->mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
tidy_finger_scroll_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  TidyFingerScrollPrivate *priv = TIDY_FINGER_SCROLL (object)->priv;
  
  switch (property_id)
    {
    case PROP_MODE :
      priv->mode = g_value_get_enum (value);
      g_object_notify (object, "mode");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
tidy_finger_scroll_dispose (GObject *object)
{
  TidyFingerScrollPrivate *priv = TIDY_FINGER_SCROLL (object)->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }
  
  if (priv->hscroll_timeline)
    {
      clutter_timeline_stop (priv->hscroll_timeline);
      g_object_unref (priv->hscroll_timeline);
      priv->hscroll_timeline = NULL;
    }
  
  if (priv->vscroll_timeline)
    {
      clutter_timeline_stop (priv->vscroll_timeline);
      g_object_unref (priv->vscroll_timeline);
      priv->vscroll_timeline = NULL;
    }
  
  if (priv->template)
    {
      g_object_unref (priv->template);
      priv->template = NULL;
    }

  G_OBJECT_CLASS (tidy_finger_scroll_parent_class)->dispose (object);
}

static void
tidy_finger_scroll_class_init (TidyFingerScrollClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyFingerScrollPrivate));

  object_class->get_property = tidy_finger_scroll_get_property;
  object_class->set_property = tidy_finger_scroll_set_property;
  object_class->dispose = tidy_finger_scroll_dispose;

  g_object_class_install_property (object_class,
                                   PROP_MODE,
                                   g_param_spec_enum ("mode",
                                                      "TidyFingerScrollMode",
                                                      "Scrolling mode",
                                                      TIDY_TYPE_FINGER_SCROLL_MODE,
                                                      TIDY_FINGER_SCROLL_MODE_PUSH,
                                                      G_PARAM_READWRITE));
}

static gboolean
motion_event_cb (ClutterActor *actor,
                 ClutterMotionEvent *event,
                 TidyFingerScroll *scroll)
{
  ClutterUnit x, y;
  
  TidyFingerScrollPrivate *priv = scroll->priv;

  if (clutter_actor_transform_stage_point (actor,
                                           CLUTTER_UNITS_FROM_DEVICE(event->x),
                                           CLUTTER_UNITS_FROM_DEVICE(event->y),
                                           &x, &y))
    {
      ClutterActor *child =
        tidy_scroll_view_get_child (TIDY_SCROLL_VIEW(scroll));
      
      if (child)
        {
          ClutterFixed dx, dy;
          TidyAdjustment *hadjust, *vadjust;
          
          guint mfreq = clutter_get_motion_events_frequency ();
          guint fps = clutter_get_default_frame_rate ();
          guint n_frames = fps / mfreq;

          tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (child),
                                           &hadjust,
                                           &vadjust);

          dx = CLUTTER_UNITS_TO_FIXED(priv->x_origin - x) +
               tidy_adjustment_get_valuex (hadjust);
          dy = CLUTTER_UNITS_TO_FIXED(priv->y_origin - y) +
               tidy_adjustment_get_valuex (vadjust);
          
          tidy_adjustment_interpolatex (hadjust, dx, n_frames, fps);
          tidy_adjustment_interpolatex (vadjust, dy, n_frames, fps);
        }
      
      priv->x_origin = x;
      priv->y_origin = y;
      
      g_get_current_time (&priv->last_motion_time);
    }

  return TRUE;
}

static void
hfade_complete_cb (ClutterActor *scrollbar, TidyFingerScroll *scroll)
{
  scroll->priv->hscroll_timeline = NULL;
}

static void
vfade_complete_cb (ClutterActor *scrollbar, TidyFingerScroll *scroll)
{
  scroll->priv->vscroll_timeline = NULL;
}

static void
show_scrollbars (TidyFingerScroll *scroll, gboolean show)
{
  ClutterActor *hscroll, *vscroll;
  TidyFingerScrollPrivate *priv = scroll->priv;
  
  /* Stop current timelines */
  if (priv->hscroll_timeline)
    {
      clutter_timeline_stop (priv->hscroll_timeline);
      g_object_unref (priv->hscroll_timeline);
    }
  
  if (priv->vscroll_timeline)
    {
      clutter_timeline_stop (priv->vscroll_timeline);
      g_object_unref (priv->vscroll_timeline);
    }

  hscroll = tidy_scroll_view_get_hscroll_bar (TIDY_SCROLL_VIEW (scroll));
  vscroll = tidy_scroll_view_get_vscroll_bar (TIDY_SCROLL_VIEW (scroll));
  
  /* Create new ones */
  if (!CLUTTER_ACTOR_IS_REACTIVE (hscroll))
    priv->hscroll_timeline = clutter_effect_fade (
                               priv->template,
                               hscroll,
                               show ? 0xFF : 0x00,
                               (ClutterEffectCompleteFunc)hfade_complete_cb,
                               scroll);

  if (!CLUTTER_ACTOR_IS_REACTIVE (vscroll))
    priv->vscroll_timeline = clutter_effect_fade (
                               priv->template,
                               vscroll,
                               show ? 0xFF : 0x00,
                               (ClutterEffectCompleteFunc)vfade_complete_cb,
                               scroll);
}

static void
deceleration_completed_cb (ClutterTimeline *timeline,
                           TidyFingerScroll *scroll)
{
  show_scrollbars (scroll, FALSE);
  g_object_unref (timeline);
  scroll->priv->deceleration_timeline = NULL;
}

static void
deceleration_new_frame_cb (ClutterTimeline *timeline,
                           gint frame_num,
                           TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *child = tidy_scroll_view_get_child (TIDY_SCROLL_VIEW(scroll));
  
  if (child)
    {
      ClutterFixed value, lower, upper, page_size;
      TidyAdjustment *hadjust, *vadjust;
      gboolean stop = TRUE;
      
      tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (child),
                                       &hadjust,
                                       &vadjust);
      
      tidy_adjustment_set_valuex (hadjust,
                                  priv->dx +
                                    tidy_adjustment_get_valuex (hadjust));
      tidy_adjustment_set_valuex (vadjust,
                                  priv->dy +
                                    tidy_adjustment_get_valuex (vadjust));
      
      /* Check if we've hit the upper or lower bounds and stop the timeline */
      tidy_adjustment_get_valuesx (hadjust, &value, &lower, &upper,
                                   NULL, NULL, &page_size);
      if (((priv->dx > 0) && (value < upper - page_size)) ||
          ((priv->dx < 0) && (value > lower)))
        stop = FALSE;
      
      if (stop)
        {
          tidy_adjustment_get_valuesx (vadjust, &value, &lower, &upper,
                                       NULL, NULL, &page_size);
          if (((priv->dy > 0) && (value < upper - page_size)) ||
              ((priv->dy < 0) && (value > lower)))
            stop = FALSE;
        }
      
      if (stop)
        {
          clutter_timeline_stop (timeline);
          deceleration_completed_cb (timeline, scroll);
        }
    }
  
  priv->dx = clutter_qdivx (priv->dx, priv->decel_rate);
  priv->dy = clutter_qdivx (priv->dy, priv->decel_rate);
}

static gboolean
button_release_event_cb (ClutterActor *actor,
                         ClutterButtonEvent *event,
                         TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv = scroll->priv;
  ClutterActor *child = tidy_scroll_view_get_child (TIDY_SCROLL_VIEW(scroll));
  gboolean decelerating = FALSE;

  if (event->button != 1)
    return FALSE;
  
  g_signal_handlers_disconnect_by_func (actor,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (actor,
                                        button_release_event_cb,
                                        scroll);
  
  clutter_ungrab_pointer ();

  if ((priv->mode == TIDY_FINGER_SCROLL_MODE_KINETIC) && (child))
    {
      ClutterUnit x, y;
      
      if (clutter_actor_transform_stage_point (actor,
                                               CLUTTER_UNITS_FROM_DEVICE(event->x),
                                               CLUTTER_UNITS_FROM_DEVICE(event->y),
                                               &x, &y))
        {
          TidyAdjustment *hadjust, *vadjust;
          ClutterUnit frac;
          GTimeVal release_time;
          glong time_diff;
          
          /* Get time delta */
          g_get_current_time (&release_time);
          
          if (priv->last_motion_time.tv_sec == release_time.tv_sec)
            time_diff = release_time.tv_usec - priv->last_motion_time.tv_usec;
          else
            time_diff = release_time.tv_usec +
                        (G_USEC_PER_SEC - priv->last_motion_time.tv_usec);
          
          /* Work out the fraction of 1/60th of a second that has elapsed */
          frac = clutter_qdivx (CLUTTER_FLOAT_TO_FIXED (time_diff/1000.0),
                                CLUTTER_FLOAT_TO_FIXED (1000.0/60.0));
          
          /* See how many units to move in 1/60th of a second */
          priv->dx = CLUTTER_UNITS_FROM_FIXED(clutter_qdivx (
                     CLUTTER_UNITS_TO_FIXED(priv->x_origin - x), frac));
          priv->dy = CLUTTER_UNITS_FROM_FIXED(clutter_qdivx (
                     CLUTTER_UNITS_TO_FIXED(priv->y_origin - y), frac));
          
          /* Get adjustments to do step-increment snapping */
          tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (child),
                                           &hadjust,
                                           &vadjust);

          if (ABS(CLUTTER_UNITS_TO_INT(priv->dx)) > 1 ||
              ABS(CLUTTER_UNITS_TO_INT(priv->dy)) > 1)
            {
              gdouble value, lower, step_increment, d, a, x, y, n;
              
              /* TODO: Convert this all to fixed point? */
              
              /* We want n, where x / y^n < z,
               * x = Distance to move per frame
               * y = Deceleration rate
               * z = maximum distance from target
               *
               * Rearrange to n = log (x / z) / log (y)
               * To simplify, z = 1, so n = log (x) / log (y)
               *
               * As z = 1, this will cause stops to be slightly abrupt - 
               * add a constant 15 frames to compensate.
               */
              x = CLUTTER_FIXED_TO_FLOAT (MAX(ABS(priv->dx), ABS(priv->dy)));
              y = CLUTTER_FIXED_TO_FLOAT (priv->decel_rate);
              n = logf (x) / logf (y) + 15.0;

              /* Now we have n, adjust dx/dy so that we finish on a step
               * boundary.
               *
               * Distance moved, using the above variable names:
               *
               * d = x + x/y + x/y^2 + ... + x/y^n
               *
               * Using geometric series,
               *
               * d = (1 - 1/y^(n+1))/(1 - 1/y)*x
               * 
               * Let a = (1 - 1/y^(n+1))/(1 - 1/y),
               *
               * d = a * x
               *
               * Find d and find its nearest page boundary, then solve for x
               *
               * x = d / a
               */
              
              /* Get adjustments, work out y^n */
              a = (1.0 - 1.0 / pow (y, n + 1)) / (1.0 - 1.0 / y);

              /* Solving for dx */
              d = a * CLUTTER_UNITS_TO_FLOAT (priv->dx);
              tidy_adjustment_get_values (hadjust, &value, &lower, NULL,
                                          &step_increment, NULL, NULL);
              d = ((rint (((value + d) - lower) / step_increment) *
                    step_increment) + lower) - value;
              priv->dx = CLUTTER_UNITS_FROM_FLOAT (d / a);

              /* Solving for dy */
              d = a * CLUTTER_UNITS_TO_FLOAT (priv->dy);
              tidy_adjustment_get_values (vadjust, &value, &lower, NULL,
                                          &step_increment, NULL, NULL);
              d = ((rint (((value + d) - lower) / step_increment) *
                    step_increment) + lower) - value;
              priv->dy = CLUTTER_UNITS_FROM_FLOAT (d / a);
              
              priv->deceleration_timeline = clutter_timeline_new ((gint)n, 60);
            }
          else
            {
              gdouble value, lower, step_increment, d, a, y;
              
              /* Start a short effects timeline to snap to the nearest step 
               * boundary (see equations above)
               */
              y = CLUTTER_FIXED_TO_FLOAT (priv->decel_rate);
              a = (1.0 - 1.0 / pow (y, 4 + 1)) / (1.0 - 1.0 / y);
              
              tidy_adjustment_get_values (hadjust, &value, &lower, NULL,
                                          &step_increment, NULL, NULL);
              d = ((rint ((value - lower) / step_increment) *
                    step_increment) + lower) - value;
              priv->dx = CLUTTER_UNITS_FROM_FLOAT (d / a);

              tidy_adjustment_get_values (vadjust, &value, &lower, NULL,
                                          &step_increment, NULL, NULL);
              d = ((rint ((value - lower) / step_increment) *
                    step_increment) + lower) - value;
              priv->dy = CLUTTER_UNITS_FROM_FLOAT (d / a);
              
              priv->deceleration_timeline = clutter_timeline_new (4, 60);
            }

          g_signal_connect (priv->deceleration_timeline, "new_frame",
                            G_CALLBACK (deceleration_new_frame_cb), scroll);
          g_signal_connect (priv->deceleration_timeline, "completed",
                            G_CALLBACK (deceleration_completed_cb), scroll);
          clutter_timeline_start (priv->deceleration_timeline);
          decelerating = TRUE;
        }
    }

  if (!decelerating)
    show_scrollbars (scroll, FALSE);
  
  /* Pass through events to children.
   * FIXME: this probably breaks click-count.
   */
  clutter_event_put ((ClutterEvent *)event);
  
  return TRUE;
}

static gboolean
after_event_cb (TidyFingerScroll *scroll)
{
  /* Check the pointer grab - if something else has grabbed it - for example,
   * a scroll-bar or some such, don't do our funky stuff.
   */
  if (clutter_get_pointer_grab () != CLUTTER_ACTOR (scroll))
    {
      g_signal_handlers_disconnect_by_func (scroll,
                                            motion_event_cb,
                                            scroll);
      g_signal_handlers_disconnect_by_func (scroll,
                                            button_release_event_cb,
                                            scroll);
    }
  
  return FALSE;
}

static gboolean
captured_event_cb (ClutterActor     *actor,
                   ClutterEvent     *event,
                   TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv = scroll->priv;
  
  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      ClutterButtonEvent *bevent = (ClutterButtonEvent *)event;
      
      if ((bevent->button == 1) &&
          (clutter_actor_transform_stage_point (actor,
                                           CLUTTER_UNITS_FROM_DEVICE(bevent->x),
                                           CLUTTER_UNITS_FROM_DEVICE(bevent->y),
                                           &priv->x_origin, &priv->y_origin)))
        {
          if (priv->deceleration_timeline)
            {
              clutter_timeline_stop (priv->deceleration_timeline);
              g_object_unref (priv->deceleration_timeline);
              priv->deceleration_timeline = NULL;
            }
          
          /* Fade in scroll-bars */
          show_scrollbars (scroll, TRUE);
          
          clutter_grab_pointer (actor);
          
          /* Add a high priority idle to check the grab after the event
           * emission is finished.
           */
          g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                           (GSourceFunc)after_event_cb,
                           scroll,
                           NULL);
          
          g_signal_connect (actor,
                            "motion-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
          g_signal_connect (actor,
                            "button-release-event",
                            G_CALLBACK (button_release_event_cb),
                            scroll);
        }
    }
  
  return FALSE;
}

static void
hscroll_notify_reactive_cb (ClutterActor     *bar,
                            GParamSpec       *pspec,
                            TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv;
  
  priv = scroll->priv;
  if (CLUTTER_ACTOR_IS_REACTIVE (bar))
    {
      if (priv->hscroll_timeline)
        {
          clutter_timeline_stop (priv->hscroll_timeline);
          g_object_unref (priv->hscroll_timeline);
          priv->hscroll_timeline = NULL;
        }
      clutter_actor_set_opacity (bar, 0xFF);
    }
}

static void
vscroll_notify_reactive_cb (ClutterActor     *bar,
                            GParamSpec       *pspec,
                            TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv;
  
  priv = scroll->priv;
  if (CLUTTER_ACTOR_IS_REACTIVE (bar))
    {
      if (priv->vscroll_timeline)
        {
          clutter_timeline_stop (priv->vscroll_timeline);
          g_object_unref (priv->vscroll_timeline);
          priv->vscroll_timeline = NULL;
        }
      clutter_actor_set_opacity (bar, 0xFF);
    }
}

static void
tidy_finger_scroll_init (TidyFingerScroll *self)
{
  ClutterActor *scrollbar;
  ClutterTimeline *effect_timeline;
  TidyFingerScrollPrivate *priv = self->priv = FINGER_SCROLL_PRIVATE (self);
  
  priv->decel_rate = CLUTTER_FLOAT_TO_FIXED(1.1f);
  
  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
  g_signal_connect (CLUTTER_ACTOR (self),
                    "captured-event",
                    G_CALLBACK (captured_event_cb),
                    self);
  
  /* Make the scroll-bars unreactive and set their opacity - we'll fade them 
   * in/out when we scroll.
   * Also, hook onto notify::reactive and don't fade in/out when the bars are 
   * set reactive (which you might want to do if you want finger-scrolling 
   * *and* a scroll bar.
   */
  scrollbar = tidy_scroll_view_get_hscroll_bar (TIDY_SCROLL_VIEW (self));
  clutter_actor_set_reactive (scrollbar, FALSE);
  clutter_actor_set_opacity (scrollbar, 0x00);
  g_signal_connect (scrollbar, "notify::reactive",
                    G_CALLBACK (hscroll_notify_reactive_cb), self);

  scrollbar = tidy_scroll_view_get_vscroll_bar (TIDY_SCROLL_VIEW (self));
  clutter_actor_set_reactive (scrollbar, FALSE);
  clutter_actor_set_opacity (scrollbar, 0x00);
  g_signal_connect (scrollbar, "notify::reactive",
                    G_CALLBACK (vscroll_notify_reactive_cb), self);
  
  effect_timeline = clutter_timeline_new_for_duration (250);
  priv->template = clutter_effect_template_new (effect_timeline,
                                                CLUTTER_ALPHA_RAMP_INC);
}

ClutterActor *
tidy_finger_scroll_new (TidyFingerScrollMode mode)
{
  return CLUTTER_ACTOR (g_object_new (TIDY_TYPE_FINGER_SCROLL,
                                      "mode", mode, NULL));
}

void
tidy_finger_scroll_stop (TidyFingerScroll *scroll)
{
  TidyFingerScrollPrivate *priv;
  
  g_return_if_fail (TIDY_IS_FINGER_SCROLL (scroll));
  
  priv = scroll->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }
}
