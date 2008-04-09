#ifndef __TWITTER_COMMON_H__
#define __TWITTER_COMMON_H__

#include <glib.h>

G_BEGIN_DECLS

#define TWITTER_ERROR   (twitter_error_quark ())

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

typedef enum {
  TWITTER_AUTH_NEGOTIATING,
  TWITTER_AUTH_RETRY,
  TWITTER_AUTH_FAILED,
  TWITTER_AUTH_SUCCESS
} TwitterAuthState;

G_END_DECLS

#endif /* __TWITTER_COMMON_H__ */
