/* tidy-scroll-bar.c: Scroll bar actor
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>
#include <clutter/cogl.h>

#include "tidy-scroll-bar.h"
#include "tidy-marshal.h"
#include "tidy-stylable.h"
#include "tidy-enum-types.h"
#include "tidy-frame.h"
#include "tidy-button.h"
#include "tidy-private.h"

static void tidy_stylable_iface_init (TidyStylableIface *iface);

G_DEFINE_TYPE_WITH_CODE (TidyScrollBar, tidy_scroll_bar, TIDY_TYPE_FRAME,
                         G_IMPLEMENT_INTERFACE (TIDY_TYPE_STYLABLE,
                                                tidy_stylable_iface_init))

#define TIDY_SCROLL_BAR_GET_PRIVATE(o) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
         TIDY_TYPE_SCROLL_BAR, \
         TidyScrollBarPrivate))

struct _TidyScrollBarPrivate
{
  TidyAdjustment  *adjustment;
  
  ClutterUnit      x_origin;
  
  ClutterActor    *handle;
};

enum
{
  PROP_0,
  PROP_HANDLE,
  PROP_ADJUSTMENT
};

static void adjustment_changed_cb (TidyAdjustment *adjustment,
                                   TidyScrollBar  *bar);

static void
tidy_scroll_bar_get_property (GObject    *gobject,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  TidyScrollBarPrivate *priv = TIDY_SCROLL_BAR (gobject)->priv;

  switch (prop_id)
    {
    case PROP_ADJUSTMENT:
      g_value_set_object (value, priv->adjustment);
      break;
    case PROP_HANDLE:
      g_value_set_object (value, priv->handle);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_scroll_bar_set_property (GObject      *gobject,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  TidyScrollBar *bar = TIDY_SCROLL_BAR (gobject);

  switch (prop_id)
    {
    case PROP_ADJUSTMENT:
      tidy_scroll_bar_set_adjustment (bar, g_value_get_object (value));
      break;
    case PROP_HANDLE:
      tidy_scroll_bar_set_handle (bar, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tidy_scroll_bar_dispose (GObject *gobject)
{
  TidyScrollBar *bar = TIDY_SCROLL_BAR (gobject);
  TidyScrollBarPrivate *priv = bar->priv;

  if (priv->adjustment)
    {
      g_object_unref (priv->adjustment);
      priv->adjustment = NULL;
    }

  if (priv->handle)
    {
      clutter_actor_destroy (priv->handle);
      priv->handle = NULL;
    }

  G_OBJECT_CLASS (tidy_scroll_bar_parent_class)->finalize (gobject);
}

static void
tidy_scroll_bar_request_coords (ClutterActor    *actor,
                                ClutterActorBox *box)
{
  TidyScrollBarPrivate *priv = TIDY_SCROLL_BAR (actor)->priv;

  /* we need the parent class to save the bounding box first */
  CLUTTER_ACTOR_CLASS (tidy_scroll_bar_parent_class)->request_coords (actor, 
                                                                      box);

  if (priv->adjustment)
    adjustment_changed_cb (priv->adjustment, TIDY_SCROLL_BAR (actor));
}

static GObject*
tidy_scroll_bar_constructor (GType                  type,
                             guint                  n_construct_properties,
                             GObjectConstructParam *construct_properties)
{
  GObject              *obj;
  TidyScrollBar        *bar;
  TidyScrollBarPrivate *priv;

  obj = G_OBJECT_CLASS (tidy_scroll_bar_parent_class)->constructor
    (type, n_construct_properties, construct_properties);

  bar =  TIDY_SCROLL_BAR (obj);
  priv = TIDY_SCROLL_BAR_GET_PRIVATE (bar);

  if (priv->handle == NULL)
    {
      /* default handle if not set on construction */
      ClutterColor *color;
      ClutterActor *rect = clutter_rectangle_new ();
  
      /* TODO: Hook onto a style-changed signal */
      tidy_stylable_get (TIDY_STYLABLE (bar), "active-color", &color, NULL);
      if (color)
        {
          clutter_rectangle_set_color (CLUTTER_RECTANGLE (rect), color);
          clutter_color_free (color);
        }

      tidy_scroll_bar_set_handle (bar, rect);
    }

  return obj;
}

static void
tidy_scroll_bar_class_init (TidyScrollBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyScrollBarPrivate));

  object_class->get_property = tidy_scroll_bar_get_property;
  object_class->set_property = tidy_scroll_bar_set_property;
  object_class->dispose      = tidy_scroll_bar_dispose;
  object_class->constructor  = tidy_scroll_bar_constructor;

  actor_class->request_coords = tidy_scroll_bar_request_coords;
  
  g_object_class_install_property 
           (object_class,
            PROP_ADJUSTMENT,
            g_param_spec_object ("adjustment",
                                 "Adjustment",
                                 "The adjustment",
                                 TIDY_TYPE_ADJUSTMENT,
                                 TIDY_PARAM_READWRITE));

  g_object_class_install_property 
           (object_class,
            PROP_HANDLE,
            g_param_spec_object ("handle",
                                 "Handle",
                                 "Actor to use for the scrollbars handle",
                                 CLUTTER_TYPE_ACTOR,
                                 TIDY_PARAM_READWRITE));
}

static void
tidy_stylable_iface_init (TidyStylableIface *iface)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      GParamSpec *pspec;

      pspec = g_param_spec_uint ("min-size",
                                 "Minimum grabber size",
                                 "Minimum size of the scroll grabber, in px",
                                 0, G_MAXUINT, 32,
                                 G_PARAM_READWRITE);
      tidy_stylable_iface_install_property (iface, 
                                            TIDY_TYPE_SCROLL_BAR, pspec);

      pspec = g_param_spec_uint ("max-size",
                                 "Maximum grabber size",
                                 "Maximum size of the scroll grabber, in px",
                                 0, G_MAXINT16, G_MAXINT16,
                                 G_PARAM_READWRITE);
      tidy_stylable_iface_install_property (iface, 
                                            TIDY_TYPE_SCROLL_BAR, pspec);
    }
}

static void
move_slider (TidyScrollBar *bar, gint x, gint y, gboolean interpolate)
{
  ClutterFixed position, lower, upper, page_size;
  ClutterUnit ux, width;
  
  TidyScrollBarPrivate *priv = bar->priv;

  if (!priv->adjustment)
    return;

  if (!clutter_actor_transform_stage_point (CLUTTER_ACTOR(bar),
                                            CLUTTER_UNITS_FROM_DEVICE(x),
                                            CLUTTER_UNITS_FROM_DEVICE(y),
                                            &ux, NULL))
    return;
  
  width = clutter_actor_get_widthu (CLUTTER_ACTOR (bar)) -
          clutter_actor_get_widthu (priv->handle);
  
  if (width == 0)
    return;
  
  ux -= priv->x_origin;
  ux = CLAMP (ux, 0, width);

  tidy_adjustment_get_valuesx (priv->adjustment,
                               NULL,
                               &lower,
                               &upper,
                               NULL,
                               NULL,
                               &page_size);

  position =
    clutter_qmulx (clutter_qdivx (CLUTTER_UNITS_TO_FIXED (ux),
                                  CLUTTER_UNITS_TO_FIXED (width)),
                   upper - lower - page_size) + lower;
  
  if (interpolate)
    {
      guint mfreq = clutter_get_motion_events_frequency ();
      guint fps = clutter_get_default_frame_rate ();
      guint n_frames = fps / mfreq;
      
      tidy_adjustment_interpolatex (priv->adjustment,
                                    position,
                                    n_frames,
                                    fps);
      return;
    }
  
  tidy_adjustment_set_valuex (priv->adjustment, position);
}

static gboolean
motion_event_cb (TidyScrollBar *bar,
                 ClutterMotionEvent *event,
                 gpointer user_data)
{
  move_slider (bar, event->x, event->y, TRUE);

  return TRUE;
}

static gboolean
button_release_event_cb (TidyScrollBar *bar,
                         ClutterButtonEvent *event,
                         gpointer user_data)
{
  if (event->button != 1)
    return FALSE;
  
  g_signal_handlers_disconnect_by_func (bar, motion_event_cb, NULL);
  g_signal_handlers_disconnect_by_func (bar, button_release_event_cb, NULL);

  move_slider (bar, event->x, event->y, FALSE);
  
  clutter_ungrab_pointer ();

  return TRUE;
}

static gboolean
button_press_event_cb (ClutterActor       *actor,
                       ClutterButtonEvent *event,
                       TidyScrollBar      *bar)
{
  TidyScrollBarPrivate *priv = bar->priv;
  
  if (event->button != 1)
    return FALSE;
  
  if (!clutter_actor_transform_stage_point (actor,
                                            CLUTTER_UNITS_FROM_DEVICE(event->x),
                                            CLUTTER_UNITS_FROM_DEVICE(event->y),
                                            &priv->x_origin, NULL))
    return FALSE;
  
  g_signal_connect_after (bar, "motion-event",
                          G_CALLBACK (motion_event_cb), NULL);
  g_signal_connect_after (bar, "button-release-event",
                          G_CALLBACK (button_release_event_cb), NULL);
  
  clutter_grab_pointer (CLUTTER_ACTOR (bar));
  
  return TRUE;
}

static void
bar_reactive_notify_cb (GObject *gobject,
                        GParamSpec *arg1,
                        gpointer user_data)
{
  TidyScrollBar *bar = TIDY_SCROLL_BAR (gobject);
  
  clutter_actor_set_reactive (bar->priv->handle,
                              clutter_actor_get_reactive (CLUTTER_ACTOR (bar)));
}



static void
tidy_scroll_bar_init (TidyScrollBar *self)
{
  TidyScrollBarPrivate *priv;
  
  priv = self->priv = TIDY_SCROLL_BAR_GET_PRIVATE (self);
}

ClutterActor *
tidy_scroll_bar_new (TidyAdjustment *adjustment)
{
  return g_object_new (TIDY_TYPE_SCROLL_BAR,
                       "adjustment", adjustment,
                       NULL);
}

ClutterActor *
tidy_scroll_bar_new_with_handle (TidyAdjustment *adjustment, 
                                 ClutterActor   *handle)
{
  return g_object_new (TIDY_TYPE_SCROLL_BAR,
                       "adjustment", adjustment,
                       "handle", handle,
                       NULL);
}


static void
tidy_scroll_bar_refresh (TidyScrollBar *bar)
{
  ClutterActor *actor = CLUTTER_ACTOR (bar);
  TidyScrollBarPrivate *priv = bar->priv;
  ClutterUnit width, button_width;
  ClutterFixed lower, upper, value, page_size;
  ClutterFixed position;
  
  /* Work out scroll bar size */
  tidy_adjustment_get_valuesx (priv->adjustment,
                               &value,
                               &lower,
                               &upper,
                               NULL,
                               NULL,
                               &page_size);

  if (upper - page_size <= lower)
    {
      clutter_actor_set_position (CLUTTER_ACTOR (priv->handle), 0, 0);
      return;
    }
  
  width = clutter_actor_get_widthu (actor);
  button_width = clutter_actor_get_widthu (priv->handle);

  position = clutter_qdivx (value - lower, upper - lower - page_size);

  /* Set padding on trough */
  clutter_actor_set_positionu (CLUTTER_ACTOR (priv->handle),
    CLUTTER_UNITS_FROM_FIXED (clutter_qmulx (position,
                              CLUTTER_UNITS_TO_FIXED (width - button_width))),
                              0);
}

static void
adjustment_changed_cb (TidyAdjustment *adjustment,
                       TidyScrollBar  *bar)
{
  TidyScrollBarPrivate *priv = bar->priv;
  ClutterUnit width, height;
  ClutterUnit real_width, real_height;
  TidyPadding padding;
  ClutterFixed lower, upper, page_size;
  ClutterFixed size, increment;
  guint min_size, max_size;
  ClutterUnit min_sizeu, max_sizeu;

  tidy_adjustment_get_valuesx (adjustment,
                               NULL,
                               &lower,
                               &upper,
                               NULL,
                               NULL,
                               &page_size);

  tidy_actor_get_padding (TIDY_ACTOR (bar), &padding);
  clutter_actor_get_sizeu (CLUTTER_ACTOR (bar), &width, &height);

  real_width = width - padding.left - padding.right;
  real_height = height - padding.top - padding.bottom;

  if (upper == lower)
    increment = CFX_ONE;
  else
    increment = clutter_qdivx (page_size, upper - lower);
  
  size = clutter_qmulx (CLUTTER_UNITS_TO_FIXED (real_width), increment);
  if (size > real_width) size = real_width;

  tidy_stylable_get (TIDY_STYLABLE (bar),
                     "min-size", &min_size,
                     "max-size", &max_size,
                     NULL);
  min_sizeu = CLUTTER_UNITS_FROM_INT (min_size);
  max_sizeu = CLUTTER_UNITS_FROM_INT (max_size);

  clutter_actor_set_sizeu (priv->handle,
                           MIN (max_sizeu,
                                MAX (min_sizeu,
                                     CLUTTER_UNITS_FROM_FIXED (size))),
                           real_height);
  
  tidy_scroll_bar_refresh (bar);
}

void
tidy_scroll_bar_set_adjustment (TidyScrollBar *bar,
                                TidyAdjustment *adjustment)
{
  TidyScrollBarPrivate *priv;
  
  g_return_if_fail (TIDY_IS_SCROLL_BAR (bar));
  
  priv = bar->priv;
  if (priv->adjustment)
    {
      g_signal_handlers_disconnect_by_func (adjustment,
                                            adjustment_changed_cb,
                                            bar);
      g_object_unref (priv->adjustment);
      priv->adjustment = NULL;
    }

  if (adjustment)
    {
      priv->adjustment = g_object_ref (adjustment);

      g_signal_connect_swapped (adjustment, "notify::value",
                                G_CALLBACK (tidy_scroll_bar_refresh),
                                bar);
      g_signal_connect (adjustment, "changed",
                        G_CALLBACK (adjustment_changed_cb),
                        bar);

      adjustment_changed_cb (adjustment, bar);
    }
}

TidyAdjustment *
tidy_scroll_bar_get_adjustment (TidyScrollBar *bar)
{
  g_return_val_if_fail (TIDY_IS_SCROLL_BAR (bar), NULL);
  
  return bar->priv->adjustment;
}

void
tidy_scroll_bar_set_handle (TidyScrollBar *bar,
                            ClutterActor *handle)
{
  TidyScrollBarPrivate *priv;
  ClutterActorBox box = { 0, 0, 0, 0 };
  
  g_return_if_fail (TIDY_IS_SCROLL_BAR (bar));
  g_return_if_fail (CLUTTER_IS_ACTOR (handle));
  
  priv = bar->priv;

  if (priv->handle == handle)
    return;

  if (priv->handle)
    {
      clutter_actor_query_coords (priv->handle, &box);
      
      g_signal_handlers_disconnect_by_func(priv->handle, 
                                           G_CALLBACK (button_press_event_cb),
                                           bar);
      g_signal_handlers_disconnect_by_func(bar, 
                                           G_CALLBACK (bar_reactive_notify_cb),
                                           NULL);

      clutter_container_remove_actor (CLUTTER_CONTAINER (bar), priv->handle);
      
      priv->handle = NULL;
    }

  priv->handle = handle;
  
  clutter_container_add_actor (CLUTTER_CONTAINER (bar), priv->handle);
  clutter_actor_show (priv->handle);
  clutter_actor_set_reactive (priv->handle, TRUE);
  clutter_actor_request_coords (priv->handle, &box);
  
  g_signal_connect_after (priv->handle, "button-press-event",
                          G_CALLBACK (button_press_event_cb), bar);
  g_signal_connect (bar, "notify::reactive",
                    G_CALLBACK (bar_reactive_notify_cb), NULL);

  g_object_notify (G_OBJECT (bar), "handle");
}

ClutterActor *
tidy_scroll_bar_get_handle (TidyScrollBar *bar)
{
  g_return_val_if_fail (TIDY_IS_SCROLL_BAR (bar), NULL);

  return bar->priv->handle;
}
