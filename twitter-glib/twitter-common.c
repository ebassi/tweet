#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <libsoup/soup.h>

#include "twitter-common.h"

guint
twitter_error_from_status (guint status)
{
  switch (status)
    {
    case SOUP_STATUS_CANT_RESOLVE:
    case SOUP_STATUS_CANT_RESOLVE_PROXY:
      return TWITTER_ERROR_HOST_NOT_FOUND;

    case SOUP_STATUS_CANCELLED:
      return TWITTER_ERROR_CANCELLED;

    case SOUP_STATUS_UNAUTHORIZED:
    case SOUP_STATUS_PAYMENT_REQUIRED:
    case SOUP_STATUS_FORBIDDEN:
      return TWITTER_ERROR_PERMISSION_DENIED;

    case SOUP_STATUS_NOT_FOUND:
    case SOUP_STATUS_GONE:
      return TWITTER_ERROR_NOT_FOUND;

    case SOUP_STATUS_GATEWAY_TIMEOUT:
      return TWITTER_ERROR_TIMED_OUT;
    }

  return TWITTER_ERROR_FAILED;
}

GQuark
twitter_error_quark (void)
{
  return g_quark_from_static_string ("twitter-error-quark");
}

gchar *
twitter_http_date_from_time_t (time_t time_)
{
  struct tm tmp;
  char buffer[256];
  gchar *retval;

#ifdef HAVE_LOCALTIME_R
  localtime_r (&time_, &tmp);
#else
  {
    struct tm *ptm = localtime (&time_);

    if (ptm == NULL)
      {
        /* Happens at least in Microsoft's C library if you pass a
         * negative time_t. Use 2000-01-01 as default date.
         */
# ifndef G_DISABLE_CHECKS
        g_return_if_fail_warning (G_LOG_DOMAIN,
                                  "twitter_http_date_from_time_t",
                                  "ptm != NULL");
# endif /* G_DISABLE_CHECKS */

        tmp.tm_mon = 0;
        tmp.tm_mday = 1;
        tmp.tm_year = 100;
      }
    else
      memcpy ((void *) &tmp, (void *) ptm, sizeof(struct tm));
  }
#endif /* HAVE_LOCALTIME_R */

  /* see RFC 1123 */
  if (strftime (buffer, sizeof (buffer), "%a, %d %b %Y %T %Z", &tmp) == 0)
    return NULL;
  else
    retval = g_strdup (buffer);

  return retval;
}

gchar *
twitter_http_date_from_delta (gint seconds)
{
  time_t now, delta;

  time (&now);
  delta = now - seconds;

  return twitter_http_date_from_time_t (delta);
}

time_t
twitter_http_date_to_time_t (const gchar *date)
{
#ifdef HAVE_STRPTIME
  struct tm tmp;

  if (strptime (date, "%a, %d %b %Y %T %Z", &tmp) == NULL)
    return mktime (&tmp);
  else
#endif /* HAVE_STRPTIME */
    return time (NULL);
}

gint
twitter_http_date_to_delta (const gchar *date)
{
  time_t res, now;

  res = twitter_http_date_to_time_t (date);
  if (res == (time_t) -1)
    return 0;

  time (&now);

  return (now - res);
}
