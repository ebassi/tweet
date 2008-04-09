#ifndef __TWITTER_POST_H__
#define __TWITTER_POST_H__

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <twitter-glib/twitter-user.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_STATUS             (twitter_status_get_type ())
#define TWITTER_STATUS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_STATUS, TwitterStatus))
#define TWITTER_IS_STATUS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_STATUS))
#define TWITTER_STATUS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_STATUS, TwitterStatusClass))
#define TWITTER_IS_STATUS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_STATUS))
#define TWITTER_STATUS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_STATUS, TwitterStatusClass))

typedef struct _TwitterStatus           TwitterStatus;
typedef struct _TwitterStatusPrivate    TwitterStatusPrivate;
typedef struct _TwitterStatusClass      TwitterStatusClass;

struct _TwitterStatus
{
  GInitiallyUnowned parent_instance;

  TwitterStatusPrivate *priv;
};

struct _TwitterStatusClass
{
  GInitiallyUnowned parent_class;
};

GType                 twitter_status_get_type       (void) G_GNUC_CONST;

TwitterStatus *       twitter_status_new            (void);
TwitterStatus *       twitter_status_new_from_data  (const gchar   *buffer);

void                  twitter_status_load_from_data (TwitterStatus *status,
                                                     const gchar   *buffer);

TwitterUser *         twitter_status_get_user       (TwitterStatus *status);
G_CONST_RETURN gchar *twitter_status_get_source     (TwitterStatus *status);
G_CONST_RETURN gchar *twitter_status_get_created_at (TwitterStatus *status);
guint                 twitter_status_get_id         (TwitterStatus *status);
gboolean              twitter_status_get_truncated  (TwitterStatus *status);
G_CONST_RETURN gchar *twitter_status_get_text       (TwitterStatus *status);

G_END_DECLS

#endif /* __TWITTER_POST_H__ */
