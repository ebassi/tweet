#ifndef __TWITTER_READER_H__
#define __TWITTER_READER_H__

#include <glib-object.h>
#include <twitter-glib/twitter-status.h>
#include <twitter-glib/twitter-timeline.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_READER             (twitter_reader_get_type ())
#define TWITTER_READER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_READER, TwitterReader))
#define TWITTER_IS_READER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_READER))
#define TWITTER_READER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_READER, TwitterReaderClass))
#define TWITTER_IS_READER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_READER))
#define TWITTER_READER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_READER, TwitterReaderClass))

typedef struct _TwitterReader           TwitterReader;
typedef struct _TwitterReaderPrivate    TwitterReaderPrivate;
typedef struct _TwitterReaderClass      TwitterReaderClass;

struct _TwitterReader
{
  GObject parent_instance;

  TwitterReaderPrivate *priv;
};

struct _TwitterReaderClass
{
  GObjectClass parent_instance;

  gboolean (* authenticate)      (TwitterReader    *reader,
                                  TwitterAuthState  state);
  void     (* timeline_received) (TwitterReader    *reader,
                                  TwitterTimeline  *timeline,
                                  const GError     *error);
  void     (* status_received)   (TwitterReader    *reader,
                                  TwitterStatus    *status,
                                  const GError     *error);
};

GType          twitter_reader_get_type             (void) G_GNUC_CONST;

TwitterReader *twitter_reader_new                  (void);
TwitterReader *twitter_reader_new_for_user         (const gchar    *email,
                                                    const gchar    *password);
void           twitter_reader_set_user             (TwitterReader  *reader,
                                                    const gchar    *email,
                                                    const gchar    *password);
void           twitter_reader_get_user             (TwitterReader  *reader,
                                                    gchar         **email,
                                                    gchar         **password);
void           twitter_reader_get_public_timeline  (TwitterReader  *reader,
                                                    guint           since_id);
void           twitter_reader_get_friends_timeline (TwitterReader  *reader,
                                                    const gchar    *friend_,
                                                    const gchar    *since_date);
void           twitter_reader_get_user_timeline    (TwitterReader  *reader,
                                                    const gchar    *user,
                                                    guint           count,
                                                    const gchar    *since_date);
void           twitter_reader_get_replies          (TwitterReader  *reader);
void           twitter_reader_show                 (TwitterReader  *reader,
                                                    guint           status_id);
G_END_DECLS

#endif /* __TWITTER_READER_H__ */
