#ifndef __TWITTER_USER_H__
#define __TWITTER_USER_H__

#include <glib-object.h>
#include <twitter-glib/twitter-common.h>

G_BEGIN_DECLS

/* TwitterUser is declared inside twitter-common.h */

GType                 twitter_user_get_type              (void) G_GNUC_CONST;

TwitterUser *         twitter_user_new                   (void);
TwitterUser *         twitter_user_new_from_data         (const gchar *buffer);

void                  twitter_user_load_from_data        (TwitterUser *user,
                                                          const gchar *buffer);

G_CONST_RETURN gchar *twitter_user_get_name              (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_url               (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_description       (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_location          (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_screen_name       (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_profile_image_url (TwitterUser *user);
guint                 twitter_user_get_id                (TwitterUser *user);
gboolean              twitter_user_get_protected         (TwitterUser *user);

TwitterStatus *       twitter_user_get_status            (TwitterUser *user);

G_END_DECLS

#endif /* __TWITTER_USER_H__ */
