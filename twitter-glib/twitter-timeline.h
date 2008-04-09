#ifndef __TWITTER_TIMELINE_H__
#define __TWITTER_TIMELINE_H__

#include <glib-object.h>
#include <twitter-glib/twitter-status.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_TIMELINE            (twitter_timeline_get_type ())
#define TWITTER_TIMELINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_TIMELINE, TwitterTimeline))
#define TWITTER_IS_TIMELINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_TIMELINE))
#define TWITTER_TIMELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_TIMELINE, TwitterTimelineClass))
#define TWITTER_IS_TIMELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_TIMELINE))
#define TWITTER_TIMELINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_TIMELINE, TwitterTimelineClass))

typedef struct _TwitterTimeline         TwitterTimeline;
typedef struct _TwitterTimelinePrivate  TwitterTimelinePrivate;
typedef struct _TwitterTimelineClass    TwitterTimelineClass;

struct _TwitterTimeline
{
  GObject parent_instance;

  TwitterTimelinePrivate *priv;
};

struct _TwitterTimelineClass
{
  GObjectClass parent_class;
};

GType            twitter_timeline_get_type       (void) G_GNUC_CONST;

TwitterTimeline *twitter_timeline_new            (void);
TwitterTimeline *twitter_timeline_new_from_data  (const gchar     *buffer);

void             twitter_timeline_load_from_data (TwitterTimeline *timeline,
                                                  const gchar     *buffer);

guint            twitter_timeline_get_count      (TwitterTimeline *timeline);
TwitterStatus *  twitter_timeline_get_status     (TwitterTimeline *timeline,
                                                  guint            id);
GList *          twitter_timeline_get_all        (TwitterTimeline *timeline);

G_END_DECLS

#endif /* __TWITTER_TIMELINE_H__ */
