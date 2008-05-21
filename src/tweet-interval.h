/* tweet-interval.h: Object holding an interval of values
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
#ifndef __TWEET_INTERVAL_H__
#define __TWEET_INTERVAL_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TWEET_TYPE_INTERVAL              (tweet_interval_get_type ())
#define TWEET_INTERVAL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_INTERVAL, TweetInterval))
#define TWEET_IS_INTERVAL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_INTERVAL))
#define TWEET_INTERVAL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_INTERVAL, TweetIntervalClass))
#define TWEET_IS_INTERVAL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_INTERVAL))
#define TWEET_INTERVAL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_INTERVAL, TweetIntervalClass))

typedef struct _TweetInterval            TweetInterval;
typedef struct _TweetIntervalPrivate     TweetIntervalPrivate;
typedef struct _TweetIntervalClass       TweetIntervalClass;

struct _TweetInterval
{
  /*< private >*/
  GInitiallyUnowned parent_instance;

  TweetIntervalPrivate *priv;
};

struct _TweetIntervalClass
{
  GInitiallyUnownedClass parent_class;
};

GType          tweet_interval_get_type           (void) G_GNUC_CONST;

TweetInterval *tweet_interval_new                (GType         gtype,
                                                  ...);
TweetInterval *tweet_interval_new_with_values    (GType         gtype,
                                                  const GValue *initial,
                                                  const GValue *final);

TweetInterval *tweet_interval_clone              (TweetInterval *interval);

GType          tweet_interval_get_value_type     (TweetInterval *interval);
void           tweet_interval_set_initial_value  (TweetInterval *interval,
                                                  const GValue *value);
void           tweet_interval_get_initial_value  (TweetInterval *interval,
                                                  GValue       *value);
GValue *       tweet_interval_peek_initial_value (TweetInterval *interval);
void           tweet_interval_set_final_value    (TweetInterval *interval,
                                                  const GValue *value);
void           tweet_interval_get_final_value    (TweetInterval *interval,
                                                  GValue       *value);
GValue *       tweet_interval_peek_final_value   (TweetInterval *interval);

void           tweet_interval_set_interval       (TweetInterval *interval,
                                                  ...);
void           tweet_interval_get_interval       (TweetInterval *interval,
                                                  ...);

G_END_DECLS

#endif /* __TWEET_INTERVAL_H__ */
