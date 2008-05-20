#ifndef __TWITTER_COMMON_H__
#define __TWITTER_COMMON_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* TwitterUser */
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
  /*< private >*/
  GInitiallyUnowned parent_instance;

  TwitterUserPrivate *priv;
};

struct _TwitterUserClass
{
  /*< private >*/
  GInitiallyUnownedClass parent_class;

  void (* changed) (TwitterUser *user);
};

/* TwitterStatus */
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
  GInitiallyUnownedClass parent_class;

  void (* changed) (TwitterStatus);
};

/**
 * TWITTER_ERROR:
 *
 * Twitter-GLib error domain.
 */
#define TWITTER_ERROR   (twitter_error_quark ())

/**
 * TwitterError:
 * @TWITTER_ERROR_HOST_NOT_FOUND:
 * @TWITTER_ERROR_CANCELLED:
 * @TWITTER_ERROR_PERMISSION_DENIED:
 * @TWITTER_ERROR_NOT_FOUND:
 * @TWITTER_ERROR_TIMED_OUT:
 * @TWITTER_ERROR_FAILED:
 *
 * Twitter error enumeration.
 */
typedef enum {
  TWITTER_ERROR_HOST_NOT_FOUND,
  TWITTER_ERROR_CANCELLED,
  TWITTER_ERROR_PERMISSION_DENIED,
  TWITTER_ERROR_NOT_FOUND,
  TWITTER_ERROR_TIMED_OUT,
  TWITTER_ERROR_FAILED
} TwitterError;

GQuark twitter_error_quark (void);

TwitterError twitter_error_from_status (guint status);

gchar *  twitter_http_date_from_time_t (time_t       time_);
gchar *  twitter_http_date_from_delta  (gint         seconds);
time_t   twitter_http_date_to_time_t   (const gchar *date);
gint     twitter_http_date_to_delta    (const gchar *date);

gboolean twitter_date_to_time_val      (const gchar *date,
                                        GTimeVal    *time_);

G_END_DECLS

#endif /* __TWITTER_COMMON_H__ */
