#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
