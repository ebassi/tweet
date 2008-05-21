/* tweet-interval.c: Object holding an interval of values
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include "tweet-interval.h"

enum
{
  PROP_0,

  PROP_VALUE_TYPE
};

#define TWEET_INTERVAL_GET_PRIVATE(obj)  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_INTERVAL, TweetIntervalPrivate))

struct _TweetIntervalPrivate
{
  GType value_type;

  GValue *values;
};

G_DEFINE_TYPE (TweetInterval, tweet_interval, G_TYPE_INITIALLY_UNOWNED);

static void
tweet_interval_finalize (GObject *gobject)
{
  TweetIntervalPrivate *priv = TWEET_INTERVAL (gobject)->priv;

  g_value_unset (&priv->values[0]);
  g_value_unset (&priv->values[1]);

  g_free (priv->values);
}

static void
tweet_interval_set_property (GObject      *gobject,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  TweetIntervalPrivate *priv = TWEET_INTERVAL_GET_PRIVATE (gobject);

  switch (prop_id)
    {
    case PROP_VALUE_TYPE:
      priv->value_type = g_value_get_gtype (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_interval_get_property (GObject    *gobject,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  TweetIntervalPrivate *priv = TWEET_INTERVAL_GET_PRIVATE (gobject);

  switch (prop_id)
    {
    case PROP_VALUE_TYPE:
      g_value_set_gtype (value, priv->value_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_interval_class_init (TweetIntervalClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (TweetIntervalPrivate));

  gobject_class->set_property = tweet_interval_set_property,
  gobject_class->get_property = tweet_interval_get_property;
  gobject_class->finalize = tweet_interval_finalize;

  pspec = g_param_spec_gtype ("value-type",
                              "Value Type",
                              "The type of the values in the interval",
                              G_TYPE_NONE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_VALUE_TYPE, pspec);
}

static void
tweet_interval_init (TweetInterval *self)
{
  TweetIntervalPrivate *priv;

  self->priv = priv = TWEET_INTERVAL_GET_PRIVATE (self);

  priv->value_type = G_TYPE_INVALID;
  priv->values = g_malloc0 (sizeof (GValue) * 2);
}

static void
tweet_interval_set_interval_valist (TweetInterval *interval,
                                    va_list        var_args)
{
  GType gtype = interval->priv->value_type;
  GValue value = { 0, };
  gchar *error;

  /* initial value */
  g_value_init (&value, gtype);
  G_VALUE_COLLECT (&value, var_args, 0, &error);
  if (error)
    {
      g_warning ("%s: %s", G_STRLOC, error);

      /* we leak the value here as it might not be in a valid state
       * given the error and calling g_value_unset() might lead to
       * undefined behaviour
       */
      g_free (error);
      return;
    }

  tweet_interval_set_initial_value (interval, &value);
  g_value_unset (&value);

  /* final value */
  g_value_init (&value, gtype);
  G_VALUE_COLLECT (&value, var_args, 0, &error);
  if (error)
    {
      g_warning ("%s: %s", G_STRLOC, error);

      /* see above */
      g_free (error);
      return;
    }

  tweet_interval_set_final_value (interval, &value);
  g_value_unset (&value);
}

static void
tweet_interval_get_interval_valist (TweetInterval *interval,
                                    va_list        var_args)
{
  GType gtype = interval->priv->value_type;
  GValue value = { 0, };
  gchar *error;

  /* initial value */
  g_value_init (&value, gtype);
  tweet_interval_get_initial_value (interval, &value);
  G_VALUE_LCOPY (&value, var_args, 0, &error);
  if (error)
    {
      g_warning ("%s: %s", G_STRLOC, error);
      g_free (error);
      g_value_unset (&value);
      return;
    }

  g_value_unset (&value);

  /* final value */
  g_value_init (&value, gtype);
  tweet_interval_get_final_value (interval, &value);
  G_VALUE_LCOPY (&value, var_args, 0, &error);
  if (error)
    {
      g_warning ("%s: %s", G_STRLOC, error);
      g_free (error);
      g_value_unset (&value);
      return;
    }

  g_value_unset (&value);
}

TweetInterval *
tweet_interval_new (GType gtype,
                    ...)
{
  TweetInterval *retval;
  va_list args;

  g_return_val_if_fail (gtype != G_TYPE_INVALID, NULL);

  retval = g_object_new (TWEET_TYPE_INTERVAL, "value-type", gtype, NULL);

  va_start (args, gtype);
  tweet_interval_set_interval_valist (retval, args);
  va_end (args);

  return retval;
}

TweetInterval *
tweet_interval_new_with_values (GType         gtype,
                                const GValue *initial,
                                const GValue *final)
{
  TweetInterval *retval;

  g_return_val_if_fail (gtype != G_TYPE_INVALID, NULL);
  g_return_val_if_fail (initial != NULL, NULL);
  g_return_val_if_fail (final != NULL, NULL);
  g_return_val_if_fail (G_VALUE_TYPE (initial) == gtype, NULL);
  g_return_val_if_fail (G_VALUE_TYPE (final) == gtype, NULL);

  retval = g_object_new (TWEET_TYPE_INTERVAL, "value-type", gtype, NULL);

  tweet_interval_set_initial_value (retval, initial);
  tweet_interval_set_final_value (retval, final);

  return retval;
}

TweetInterval *
tweet_interval_clone (TweetInterval *interval)
{
  TweetInterval *retval;
  GType gtype;

  g_return_val_if_fail (TWEET_IS_INTERVAL (interval), NULL);
  g_return_val_if_fail (interval->priv->value_type != G_TYPE_INVALID, NULL);

  gtype = interval->priv->value_type;
  retval = g_object_new (TWEET_TYPE_INTERVAL, "value-type", gtype, NULL);

  tweet_interval_set_initial_value (retval,
                                    tweet_interval_peek_initial_value (interval));
  tweet_interval_set_final_value (retval,
                                  tweet_interval_peek_final_value (interval));

  return retval;
}

GType
tweet_interval_get_value_type (TweetInterval *interval)
{
  g_return_val_if_fail (TWEET_IS_INTERVAL (interval), G_TYPE_INVALID);

  return interval->priv->value_type;
}

void
tweet_interval_set_initial_value (TweetInterval *interval,
                                  const GValue  *value)
{
  TweetIntervalPrivate *priv;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (value != NULL);

  priv = interval->priv;

  if (G_IS_VALUE (&priv->values[0]))
    g_value_unset (&priv->values[0]);

  g_value_init (&priv->values[0], priv->value_type);
  g_value_copy (value, &priv->values[0]);
}

void
tweet_interval_get_initial_value (TweetInterval *interval,
                                  GValue        *value)
{
  TweetIntervalPrivate *priv;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (value != NULL);

  priv = interval->priv;

  g_value_copy (&priv->values[0], value);
}

GValue *
tweet_interval_peek_initial_value (TweetInterval *interval)
{
  g_return_val_if_fail (TWEET_IS_INTERVAL (interval), NULL);

  return interval->priv->values;
}

void
tweet_interval_set_final_value (TweetInterval *interval,
                                const GValue  *value)
{
  TweetIntervalPrivate *priv;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (value != NULL);

  priv = interval->priv;

  if (G_IS_VALUE (&priv->values[1]))
    g_value_unset (&priv->values[1]);

  g_value_init (&priv->values[1], priv->value_type);
  g_value_copy (value, &priv->values[1]);
}

void
tweet_interval_get_final_value (TweetInterval *interval,
                                GValue        *value)
{
  TweetIntervalPrivate *priv;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (value != NULL);

  priv = interval->priv;

  g_value_copy (&priv->values[1], value);
}

GValue *
tweet_interval_peek_final_value (TweetInterval *interval)
{
  g_return_val_if_fail (TWEET_IS_INTERVAL (interval), NULL);

  return interval->priv->values + 1;
}

void
tweet_interval_set_interval (TweetInterval *interval,
                             ...)
{
  va_list args;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (interval->priv->value_type != G_TYPE_INVALID);

  va_start (args, interval);
  tweet_interval_set_interval_valist (interval, args);
  va_end (args);
}

void
tweet_interval_get_interval (TweetInterval *interval,
                             ...)
{
  va_list args;

  g_return_if_fail (TWEET_IS_INTERVAL (interval));
  g_return_if_fail (interval->priv->value_type != G_TYPE_INVALID);

  va_start (args, interval);
  tweet_interval_get_interval_valist (interval, args);
  va_end (args);
}
