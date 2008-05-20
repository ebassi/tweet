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
