#ifndef __TWEET_PREFERENCES_H__
#define __TWEET_PREFERENCES_H__

#include <gtk/gtk.h>
#include "tweet-config.h"

G_BEGIN_DECLS

void tweet_show_preferences_dialog (GtkWindow   *parent,
                                    const gchar *title,
                                    TweetConfig *config);

G_END_DECLS

#endif /* __TWEET_PREFERENCES_H__ */
