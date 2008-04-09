#ifndef __TWITTER_USER_H__
#define __TWITTER_USER_H__

#include <glib-object.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_USER               (twitter_user_get_type ())
#define TWITTER_USER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_USER, TwitterUser))
#define TWITTER_IS_USER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_USER))
#define TWITTER_USER_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_USER, TwitterUserClass))
#define TWITTER_IS_USER_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_USER))
#define TWITTER_USER_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_USER, TwitterUserClass))

typedef struct _TwitterUser             TwitterUser;
typedef struct _TwitterUserPrivate      TwitterUserPrivate;
typedef struct _TwitterUserClass        TwitterUserClass;

struct _TwitterUser
{
  GInitiallyUnowned parent_instance;

  TwitterUserPrivate *priv;
};

struct _TwitterUserClass
{
  GInitiallyUnownedClass parent_class;
};

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

G_END_DECLS

#endif /* __TWITTER_USER_H__ */
