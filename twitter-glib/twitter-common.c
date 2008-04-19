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
  SoupDate *soup_date;
  gchar *retval;

  soup_date = soup_date_new_from_time_t (time_);
  retval = soup_date_to_string (soup_date, SOUP_DATE_HTTP);
  soup_date_free (soup_date);

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
  SoupDate *soup_date;
  time_t retval;

  soup_date = soup_date_new_from_string (date);
  retval = soup_date_to_time_t (soup_date);
  soup_date_free (soup_date);

  return retval;
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
