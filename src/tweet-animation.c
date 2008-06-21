/* tweet-animation.c: Simple animation API
 *
 * This file is part of Tweet.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>

#include <clutter/clutter.h>

#include "tweet-animation.h"
#include "tweet-enum-types.h"
#include "tweet-interval.h"

#define I_(str)         (g_intern_static_string ((str)))

enum
{
  PROP_0,

  PROP_ACTOR,
  PROP_MODE,
  PROP_DURATION,
  PROP_LOOP
};

enum
{
  COMPLETED,

  LAST_SIGNAL
};

#define TWEET_ANIMATION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_ANIMATION, TweetAnimationPrivate))

struct _TweetAnimationPrivate
{
  ClutterActor *actor;

  GHashTable *properties;

  TweetAnimationMode mode;

  guint loop : 1;
  guint duration;
  ClutterTimeline *timeline;
  guint timeline_completed_id;

  ClutterAlpha *alpha;
  guint alpha_notify_id;
};

static guint animation_signals[LAST_SIGNAL] = { 0, };

static GQuark quark_actor_animations = 0;

G_DEFINE_TYPE (TweetAnimation, tweet_animation, G_TYPE_INITIALLY_UNOWNED);

static void
tweet_animation_finalize (GObject *gobject)
{
  TweetAnimationPrivate *priv = TWEET_ANIMATION (gobject)->priv;

  g_hash_table_destroy (priv->properties);

  G_OBJECT_CLASS (tweet_animation_parent_class)->finalize (gobject);
}

static void
tweet_animation_dispose (GObject *gobject)
{
  TweetAnimationPrivate *priv = TWEET_ANIMATION (gobject)->priv;

  if (priv->actor)
    {
      g_object_unref (priv->actor);
      priv->actor = NULL;
    }

  if (priv->timeline)
    {
      if (priv->timeline_completed_id)
        {
          g_signal_handler_disconnect (priv->timeline, priv->timeline_completed_id);
          priv->timeline_completed_id = 0;
        }

      g_object_unref (priv->timeline);
      priv->timeline = NULL;
    }

  if (priv->alpha)
    {
      if (priv->alpha_notify_id)
        {
          g_signal_handler_disconnect (priv->alpha, priv->alpha_notify_id);
          priv->alpha_notify_id = 0;
        }

      g_object_unref (priv->alpha);
      priv->alpha = NULL;
    }

  G_OBJECT_CLASS (tweet_animation_parent_class)->dispose (gobject);
}

static void
tweet_animation_set_property (GObject      *gobject,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  TweetAnimation *animation = TWEET_ANIMATION (gobject);

  switch (prop_id)
    {
    case PROP_ACTOR:
      tweet_animation_set_actor (animation, g_value_get_object (value));
      break;

    case PROP_MODE:
      tweet_animation_set_mode (animation, g_value_get_enum (value));
      break;

    case PROP_DURATION:
      tweet_animation_set_duration (animation, g_value_get_uint (value));
      break;

    case PROP_LOOP:
      tweet_animation_set_loop (animation, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_animation_get_property (GObject    *gobject,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  TweetAnimationPrivate *priv = TWEET_ANIMATION (gobject)->priv;

  switch (prop_id)
    {
    case PROP_ACTOR:
      g_value_set_object (value, priv->actor);
      break;

    case PROP_MODE:
      g_value_set_enum (value, priv->mode);
      break;

    case PROP_DURATION:
      g_value_set_uint (value, priv->duration);
      break;

    case PROP_LOOP:
      g_value_set_boolean (value, priv->loop);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_animation_class_init (TweetAnimationClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  quark_actor_animations = g_quark_from_static_string ("tweet-actor-animations");

  g_type_class_add_private (klass, sizeof (TweetAnimationPrivate));

  gobject_class->set_property = tweet_animation_set_property;
  gobject_class->get_property = tweet_animation_get_property;
  gobject_class->dispose = tweet_animation_dispose;
  gobject_class->finalize = tweet_animation_finalize;

  pspec = g_param_spec_object ("actor",
                               "Actor",
                               "Actor to which the animation applies to",
                               CLUTTER_TYPE_ACTOR,
                               G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_ACTOR, pspec);

  pspec = g_param_spec_enum ("mode",
                             "Mode",
                             "The mode of the animation",
                             TWEET_TYPE_ANIMATION_MODE,
                             TWEET_LINEAR,
                             G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_MODE, pspec);

  pspec = g_param_spec_uint ("duration",
                             "Duration",
                             "Duration of the animation, in milliseconds",
                             0, G_MAXUINT, 0,
                             G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_DURATION, pspec);

  pspec = g_param_spec_boolean ("loop",
                                "Loop",
                                "Whether the animation should loop",
                                FALSE,
                                G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_LOOP, pspec);

  animation_signals[COMPLETED] =
    g_signal_new (I_("completed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TweetAnimationClass, completed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
tweet_animation_init (TweetAnimation *self)
{
  self->priv = TWEET_ANIMATION_GET_PRIVATE (self);

  self->priv->mode = TWEET_LINEAR;
  self->priv->properties =
    g_hash_table_new_full (g_str_hash, g_str_equal,
                           (GDestroyNotify) g_free,
                           (GDestroyNotify) g_object_unref);
}

TweetAnimation *
tweet_animation_new (void)
{
  return g_object_new (TWEET_TYPE_ANIMATION, NULL);
}

void
tweet_animation_set_actor (TweetAnimation *animation,
                           ClutterActor   *actor)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));
  g_return_if_fail (CLUTTER_IS_ACTOR (actor));

  priv = animation->priv;

  if (priv->actor)
    g_object_unref (priv->actor);

  priv->actor = g_object_ref (actor);

  g_object_notify (G_OBJECT (animation), "actor");
}

ClutterActor *
tweet_animation_get_actor (TweetAnimation *animation)
{
  g_return_val_if_fail (TWEET_IS_ANIMATION (animation), NULL);

  return animation->priv->actor;
}

void
tweet_animation_set_mode (TweetAnimation     *animation,
                          TweetAnimationMode  mode)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));

  priv = animation->priv;

  if (priv->mode != mode)
    {
      priv->mode = mode;

      if (priv->alpha)
        {
          switch (priv->mode)
            {
            case TWEET_LINEAR:
              clutter_alpha_set_func (priv->alpha,
                                      CLUTTER_ALPHA_RAMP_INC,
                                      NULL, NULL);
              break;

            case TWEET_SINE:
              clutter_alpha_set_func (priv->alpha,
                                      CLUTTER_ALPHA_SINE_INC,
                                      NULL, NULL);
              break;

            default:
              g_assert_not_reached ();
              break;
            }
        }

      g_object_notify (G_OBJECT (animation), "mode");
    }
}

TweetAnimationMode
tweet_animation_get_mode (TweetAnimation *animation)
{
  g_return_val_if_fail (TWEET_IS_ANIMATION (animation), TWEET_LINEAR);

  return animation->priv->mode;
}

void
tweet_animation_set_duration (TweetAnimation *animation,
                              gint           msecs)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));

  priv = animation->priv;

  if (priv->duration != msecs)
    {
      priv->duration = msecs;

      if (priv->timeline)
        clutter_timeline_set_duration (priv->timeline, msecs);

      g_object_notify (G_OBJECT (animation), "duration");
    }
}

void
tweet_animation_set_loop (TweetAnimation *animation,
                          gboolean        loop)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));

  priv = animation->priv;

  if (priv->loop != loop)
    {
      priv->loop = loop;

      if (priv->timeline)
        clutter_timeline_set_loop (priv->timeline, priv->loop);

      g_object_notify (G_OBJECT (animation), "loop");
    }
}

gboolean
tweet_animation_get_loop (TweetAnimation *animation)
{
  g_return_val_if_fail (TWEET_IS_ANIMATION (animation), FALSE);

  return animation->priv->loop;
}

guint
tweet_animation_get_duration (TweetAnimation *animation)
{
  g_return_val_if_fail (TWEET_IS_ANIMATION (animation), 0);

  return animation->priv->duration;
}

/*
 * tweet_interval_compute_value:
 * @interval: a #TweetInterval
 * @factor: the progress factor, between 0 and %CLUTTER_ALPHA_MAX_ALPHA
 * @value: return location for an initialized #GValue
 *
 * Computes the value between the @interval boundaries fiven the
 * progress @factor and puts it into @value.
 */
static void
tweet_interval_compute_value (TweetInterval *interval,
                              guint32        factor,
                              GValue        *value)
{
  GValue *initial, *final;
  GType value_type;

  initial = tweet_interval_peek_initial_value (interval);
  final = tweet_interval_peek_final_value (interval);

  value_type = tweet_interval_get_value_type (interval);

  switch (G_TYPE_FUNDAMENTAL (value_type))
    {
    case G_TYPE_INT:
      {
        gint ia, ib, res;

        ia = g_value_get_int (initial);
        ib = g_value_get_int (final);

        res = factor * (ib - ia)
            / CLUTTER_ALPHA_MAX_ALPHA
            + ia;

        g_value_set_int (value, res);
      }
      break;

    case G_TYPE_UINT:
      {
        guint ia, ib, res;

        ia = g_value_get_uint (initial);
        ib = g_value_get_uint (final);

        res = factor * (ib - ia)
            / CLUTTER_ALPHA_MAX_ALPHA
            + ia;

        g_value_set_uint (value, res);
      }
      break;

    case G_TYPE_UCHAR:
      {
        guchar ia, ib, res;

        ia = g_value_get_uchar (initial);
        ib = g_value_get_uchar (final);

        res = factor * (ib - ia)
            / CLUTTER_ALPHA_MAX_ALPHA
            + ia;

        g_value_set_uchar (value, res);
      }
      break;

    case G_TYPE_FLOAT:
    case G_TYPE_DOUBLE:
      {
        gdouble ia, ib, res;

        ia = g_value_get_double (initial);
        ib = g_value_get_double (final);

        res = factor * (ib - ia)
            / CLUTTER_ALPHA_MAX_ALPHA
            + ia;

        if (value_type == G_TYPE_DOUBLE)
          g_value_set_double (value, res);
        else
          g_value_set_float (value, res);
      }
      break;

    case G_TYPE_BOOLEAN:
      if (factor > CLUTTER_ALPHA_MAX_ALPHA / 2)
        g_value_set_boolean (value, TRUE);
      else
        g_value_set_boolean (value, FALSE);
      break;

    default:
      break;
    }
}

static gboolean
tweet_interval_check_bounds (TweetInterval *interval,
                            GParamSpec   *pspec)
{
  GType pspec_gtype = G_PARAM_SPEC_VALUE_TYPE (pspec);

  switch (G_TYPE_FUNDAMENTAL (pspec_gtype))
    {
    case G_TYPE_INT:
      {
        GParamSpecInt *pspec_int = G_PARAM_SPEC_INT (pspec);
        gint initial, final;

        tweet_interval_get_interval (interval, &initial, &final);
        if ((initial >= pspec_int->minimum && initial <= pspec_int->maximum) &&
            (final >= pspec_int->minimum && final <= pspec_int->maximum))
          return TRUE;
        else
          return FALSE;
      }
      break;

    case G_TYPE_UINT:
      {
        GParamSpecUInt *pspec_uint = G_PARAM_SPEC_UINT (pspec);
        gint initial, final;

        tweet_interval_get_interval (interval, &initial, &final);
        if ((initial >= pspec_uint->minimum && initial <= pspec_uint->maximum) &&
            (final >= pspec_uint->minimum && final <= pspec_uint->maximum))
          return TRUE;
        else
          return FALSE;
      }
      break;

    case G_TYPE_UCHAR:
      {
        GParamSpecUChar *pspec_uchar = G_PARAM_SPEC_UCHAR (pspec);
        guchar initial, final;

        tweet_interval_get_interval (interval, &initial, &final);
        if ((initial >= pspec_uchar->minimum && initial <= pspec_uchar->maximum) &&
            (final >= pspec_uchar->minimum && final <= pspec_uchar->maximum))
          return TRUE;
        else
          return FALSE;
      }
      break;

    case G_TYPE_BOOLEAN:
      return TRUE;

    default:
      break;
    }

  return TRUE;
}

void
tweet_animation_bind_property (TweetAnimation *animation,
                              const gchar   *property_name,
                              TweetInterval  *interval)
{
  TweetAnimationPrivate *priv;
  GObjectClass *klass;
  GParamSpec *pspec;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));
  g_return_if_fail (property_name != NULL);
  g_return_if_fail (TWEET_IS_INTERVAL (interval));

  priv = animation->priv;

  if (G_UNLIKELY (!priv->actor))
    {
      g_warning ("Cannot bind property `%s': the animation has no "
                 "actor set. You need to call tweet_animation_set_actor() "
                 "first to be able to bind a property",
                 property_name);
      return;
    }

  if (G_UNLIKELY (tweet_animation_has_property (animation, property_name)))
    {
      g_warning ("Cannot bind property `%s': the animation already has "
                 "a bound property with the same name",
                 property_name);
      return;
    }

  klass = G_OBJECT_GET_CLASS (priv->actor);
  pspec = g_object_class_find_property (klass, property_name);
  if (!pspec)
    {
      g_warning ("Cannot bind property `%s': actors of type `%s' have "
                 "no such property",
                 property_name,
                 g_type_name (G_OBJECT_TYPE (priv->actor)));
      return;
    }

  if (!(pspec->flags & G_PARAM_WRITABLE))
    {
      g_warning ("Cannot bind property `%s': the property is not writable", 
                 property_name);
      return;
    }

  if (!g_value_type_compatible (G_PARAM_SPEC_VALUE_TYPE (pspec),
                                tweet_interval_get_value_type (interval)))
    {
      g_warning ("Cannot bind property `%s': the interval value of "
                 "type `%s' is not compatible with the property value "
                 "of type `%s'",
                 property_name,
                 g_type_name (tweet_interval_get_value_type (interval)),
                 g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)));
      return;
    }

  if (!tweet_interval_check_bounds (interval, pspec))
    {
      g_warning ("Cannot bind property `%s': the interval is out "
                 "of bounds",
                 property_name);
      return;
    }

  g_hash_table_insert (priv->properties,
                       g_strdup (property_name),
                       g_object_ref_sink (interval));
}

void
tweet_animation_unbind_property (TweetAnimation *animation,
                                const gchar   *property_name)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));
  g_return_if_fail (property_name != NULL);

  priv = animation->priv;

  if (!tweet_animation_has_property (animation, property_name))
    {
      g_warning ("Cannot unbind property `%s': the animation has "
                 "no bound property with that name",
                 property_name);
      return;
    }

  g_hash_table_remove (priv->properties, property_name);
}

gboolean
tweet_animation_has_property (TweetAnimation *animation,
                             const gchar   *property_name)
{
  g_return_val_if_fail (TWEET_IS_ANIMATION (animation), FALSE);
  g_return_val_if_fail (property_name != NULL, FALSE);

  return g_hash_table_lookup (animation->priv->properties, property_name) != NULL;
}

static void
on_timeline_completed (ClutterTimeline *timeline,
                       TweetAnimation   *animation)
{
  g_signal_emit (animation, animation_signals[COMPLETED], 0);
}

static void
on_alpha_notify (GObject        *gobject,
                 GParamSpec     *pspec,
                 TweetAnimation *animation)
{
  TweetAnimationPrivate *priv = animation->priv;
  GList *properties, *p;
  guint32 alpha_value;

  alpha_value = clutter_alpha_get_alpha (CLUTTER_ALPHA (gobject));

  g_object_freeze_notify (G_OBJECT (priv->actor));

  properties = g_hash_table_get_keys (priv->properties);
  for (p = properties; p != NULL; p = p->next)
    {
      const gchar *p_name = p->data;
      TweetInterval *interval = g_hash_table_lookup (priv->properties, p_name);
      GValue value = { 0, };

      g_assert (TWEET_IS_INTERVAL (interval));

      g_value_init (&value, tweet_interval_get_value_type (interval));

      tweet_interval_compute_value (interval, alpha_value, &value);

      g_object_set_property (G_OBJECT (priv->actor), p_name, &value);

      g_value_unset (&value);
    }

  g_list_free (properties);

  g_object_thaw_notify (G_OBJECT (priv->actor));
}

void
tweet_animation_start (TweetAnimation *animation)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));

  priv = animation->priv;

  if (!priv->timeline)
    priv->timeline = clutter_timeline_new_for_duration (priv->duration);

  if (!priv->alpha)
    {
      priv->alpha = clutter_alpha_new ();
      g_object_ref_sink (priv->alpha);

      clutter_alpha_set_timeline (priv->alpha, priv->timeline);
    }

  switch (priv->mode)
    {
    case TWEET_LINEAR:
      clutter_alpha_set_func (priv->alpha,
                              CLUTTER_ALPHA_RAMP_INC,
                              NULL, NULL);
      break;

    case TWEET_SINE:
      clutter_alpha_set_func (priv->alpha,
                              CLUTTER_ALPHA_SINE_INC,
                              NULL, NULL);
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  if (!priv->timeline_completed_id)
    priv->timeline_completed_id =
      g_signal_connect (priv->timeline, "completed",
                        G_CALLBACK (on_timeline_completed),
                        animation);

  if (!priv->alpha_notify_id)
    priv->alpha_notify_id = g_signal_connect (priv->alpha, "notify::alpha",
                                              G_CALLBACK (on_alpha_notify),
                                              animation);

  clutter_timeline_start (priv->timeline);
}

void
tweet_animation_stop (TweetAnimation *animation)
{
  TweetAnimationPrivate *priv;

  g_return_if_fail (TWEET_IS_ANIMATION (animation));

  priv = animation->priv;

  if (!priv->timeline)
    return;

  clutter_timeline_stop (priv->timeline);
}

static void
on_animation_complete (TweetAnimation *animation,
                       ClutterActor   *actor)
{
  GList *animations;

  /* do not remove the animation if it is looping */
  if (animation->priv->loop)
    return;

  animations = g_object_get_qdata (G_OBJECT (actor), quark_actor_animations);
  if (animations)
    {
      animations = g_list_remove (animations, animation);
      g_object_set_qdata (G_OBJECT (actor), quark_actor_animations, animations);
    }

  g_object_unref (animation);
}

/*
 * tweet_actor_animate:
 * @actor: a #ClutterActor
 * @mode: a #TweetAnimationMode value
 * @duration: duration of the animation, in milliseconds
 * @first_property_name: the name of a property
 * @VarArgs: a %NULL terminated list of properties and #TweetInterval<!-- -->s
 *   pairs
 *
 * Animates the given list of properties of @actor between two values of
 * an interval set for each property. The animation has a definite duration
 * and a speed given by the @mode.
 *
 * For example, this:
 *
 * |[
 *   tweet_actor_animate (rectangle, TWEET_LINEAR, 250,
 *                        "width", tweet_interval_new (G_TYPE_UINT, 1, 100),
 *                        "height", tweet_interval_new (G_TYPE_UINT, 1, 100),
 *                        NULL);
 * ]|
 *
 * will make width and height properties of the #ClutterActor "rectangle"
 * grow linearly between the values of 1 and 100 pixels.
 *
 * This function will implicitly create a #TweetAnimation object which
 * will be assigned to the @actor and will be returned to the developer
 * to control the animation or to know when the animation has been
 * completed.
 *
 * <note>Unless the animation is looping, it will become invalid as soon
 * as it is complete.</note>
 *
 * Return value: a #TweetAnimation object. The object is owned by the
 *   #ClutterActor and should not be unreferenced with g_object_unref()
 */
TweetAnimation *
tweet_actor_animate (ClutterActor       *actor,
                     TweetAnimationMode  mode,
                     guint               duration,
                     const gchar        *first_property_name,
                     ...)
{
  TweetAnimation *animation;
  va_list var_args;
  const gchar *property_name;
  GList *animations;

  g_return_val_if_fail (CLUTTER_IS_ACTOR (actor), NULL);
  g_return_val_if_fail (duration > 0, NULL);
  g_return_val_if_fail (first_property_name != NULL, NULL);

  animation = tweet_animation_new ();
  tweet_animation_set_actor (animation, actor);
  tweet_animation_set_duration (animation, duration);
  tweet_animation_set_mode (animation, mode);

  va_start (var_args, first_property_name);

  property_name = first_property_name;
  while (property_name != NULL)
    {
      TweetInterval *interval;

      interval = va_arg (var_args, TweetInterval*);
      g_return_val_if_fail (TWEET_IS_INTERVAL (interval), animation);

      tweet_animation_bind_property (animation, property_name, interval);

      property_name = va_arg (var_args, gchar*);
    }

  va_end (var_args);

  animations = g_object_get_qdata (G_OBJECT (actor), quark_actor_animations);
  animations = g_list_prepend (animations, g_object_ref_sink (animation));
  g_object_set_qdata (G_OBJECT (actor), quark_actor_animations, animations);

  g_signal_connect (animation,
                    "completed", G_CALLBACK (on_animation_complete),
                    actor);

  tweet_animation_start (animation);

  return animation;
}
