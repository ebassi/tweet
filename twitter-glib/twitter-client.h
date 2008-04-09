#ifndef __TWITTER_CLIENT_H__
#define __TWITTER_CLIENT_H__

#include <glib-object.h>
#include <twitter-glib/twitter-status.h>
#include <twitter-glib/twitter-timeline.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_CLIENT             (twitter_client_get_type ())
#define TWITTER_CLIENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_CLIENT, TwitterClient))
#define TWITTER_IS_CLIENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_CLIENT))
#define TWITTER_CLIENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_CLIENT, TwitterClientClass))
#define TWITTER_IS_CLIENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_CLIENT))
#define TWITTER_CLIENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_CLIENT, TwitterClientClass))

typedef struct _TwitterClient           TwitterClient;
typedef struct _TwitterClientPrivate    TwitterClientPrivate;
typedef struct _TwitterClientClass      TwitterClientClass;

struct _TwitterClient
{
  GObject parent_instance;

  TwitterClientPrivate *priv;
};

struct _TwitterClientClass
{
  GObjectClass parent_instance;

  gboolean (* authenticate)      (TwitterClient    *client,
                                  TwitterAuthState  state);
  void     (* timeline_received) (TwitterClient    *client,
                                  TwitterTimeline  *timeline,
                                  const GError     *error);
  void     (* status_received)   (TwitterClient    *client,
                                  TwitterStatus    *status,
                                  const GError     *error);
};

GType          twitter_client_get_type             (void) G_GNUC_CONST;

TwitterClient *twitter_client_new                  (void);
TwitterClient *twitter_client_new_for_user         (const gchar    *email,
                                                    const gchar    *password);
void           twitter_client_set_user             (TwitterClient  *client,
                                                    const gchar    *email,
                                                    const gchar    *password);
void           twitter_client_get_user             (TwitterClient  *client,
                                                    gchar         **email,
                                                    gchar         **password);
void           twitter_client_get_public_timeline  (TwitterClient  *client,
                                                    guint           since_id);
void           twitter_client_get_friends_timeline (TwitterClient  *client,
                                                    const gchar    *friend_,
                                                    const gchar    *since_date);
void           twitter_client_get_user_timeline    (TwitterClient  *client,
                                                    const gchar    *user,
                                                    guint           count,
                                                    const gchar    *since_date);
void           twitter_client_get_replies          (TwitterClient  *client);
void           twitter_client_show                 (TwitterClient  *client,
                                                    guint           status_id);
G_END_DECLS

#endif /* __TWITTER_CLIENT_H__ */
