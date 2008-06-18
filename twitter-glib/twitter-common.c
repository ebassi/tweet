/* twitter-common.c: Common definitions
 *
 * This file is part of Twitter-GLib.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

    case SOUP_STATUS_NOT_MODIFIED:
      return TWITTER_ERROR_NOT_MODIFIED;
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

  g_return_val_if_fail (date != NULL, (time_t) -1);

  soup_date = soup_date_new_from_string (date);
  if (!soup_date)
    return (time_t) -1;

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

gboolean
twitter_date_to_time_val (const gchar *date,
                          GTimeVal    *time_)
{
  time_t res;
  SoupDate *soup_date;

  g_return_val_if_fail (date != NULL, FALSE);
  g_return_val_if_fail (time_ != NULL, FALSE);

  soup_date = soup_date_new_from_string (date);
  if (soup_date)
    {
      res = soup_date_to_time_t (soup_date);
      soup_date_free (soup_date);

      time_->tv_sec = res;
      time_->tv_usec = 0;

      return TRUE;
    }

#ifdef HAVE_STRPTIME
  {
    struct tm tmp;

    /* OMFG, ctime()? really? what are they? insane? and it's already been
     * adjusted to the user settings instead of giving us the time of the
     * status when it was sent
     */
    strptime (date, "%a %b %d %T %z %Y", &tmp);

    res = mktime (&tmp);
    if (res != 0)
      {
        time_t now_t;
        struct tm now_tm;

        time (&now_t);
        localtime_r (&now_t, &now_tm);

        /* twitter blissfully ignores the existence of the
         * daylight saving time
         */
        if (now_tm.tm_isdst)
          res += 3600;

        time_->tv_sec = res;
        time_->tv_usec = 0;

        return TRUE;
      }
  }
#endif /* HAVE_STRPTIME */

  return FALSE;
}
