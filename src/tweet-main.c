/* tweet-main.c: Main entry point
 *
 * This file is part of Tweet.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include "tweet-app.h"

int
main (int   argc,
      char *argv[])
{
  TweetApp *app;
  GError *error;
  int res = EXIT_FAILURE;

  error = NULL;
  app = tweet_app_get_default (&argc, &argv, &error);
  if (error)
    {
      g_warning ("Unable to launch %s: %s", PACKAGE, error->message);
      g_error_free (error);
      return res;
    }

  if (tweet_app_is_running (app))
    res = EXIT_SUCCESS;
  else
    res = tweet_app_run (app);

  g_object_unref (app);

  return res;
}
