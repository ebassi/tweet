#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
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

