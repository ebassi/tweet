#ifndef __TWITTER_API_H__
#define __TWITTER_API_H__

#include <glib.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

SoupMessage *twitter_api_public_timeline    (gint         since_id);
SoupMessage *twitter_api_friends_timeline   (const gchar *user,
                                             const gchar *since);
SoupMessage *twitter_api_user_timeline      (const gchar *user,
                                             guint        count,
                                             const gchar *since);
SoupMessage *twitter_api_status_show        (guint        status_id);
SoupMessage *twitter_api_update             (const gchar *text);
SoupMessage *twitter_api_replies            (void);
SoupMessage *twitter_api_destroy            (guint        status_id);
SoupMessage *twitter_api_friends            (const gchar *user,
                                             gint         page,
                                             gboolean     lite);
SoupMessage *twitter_api_featured           (void);
SoupMessage *twitter_api_user_show          (const gchar *user);
SoupMessage *twitter_api_verify_credentials (void);
SoupMessage *twitter_api_end_session        (void);
SoupMessage *twitter_api_followers          (gint         page,
                                             gboolean     lite);
SoupMessage *twitter_api_create_friend      (const gchar *user);
SoupMessage *twitter_api_destroy_friend     (const gchar *user);
SoupMessage *twitter_api_favorites          (const gchar *user,
                                             gint         page);
SoupMessage *twitter_api_create_favorite    (guint        status_id);
SoupMessage *twitter_api_destroy_favorite   (guint        status_id);
SoupMessage *twitter_api_follow             (const gchar *user);
SoupMessage *twitter_api_leave              (const gchar *user);
SoupMessage *twitter_api_archive            (gint         page);

G_END_DECLS

#endif /* __TWITTER_API_H__ */
