/* tweet-config.h: Configuration object
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
#ifndef __TWEET_CONFIG_H__
#define __TWEET_CONFIG_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TWEET_TYPE_CONFIG               (tweet_config_get_type ())
#define TWEET_CONFIG(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_CONFIG, TweetConfig))
#define TWEET_IS_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_CONFIG))
#define TWEET_CONFIG_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_CONFIG, TweetConfigClass))
#define TWEET_IS_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_CONFIG))
#define TWEET_CONFIG_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_CONFIG, TweetConfigClass))

typedef struct _TweetConfig             TweetConfig;
typedef struct _TweetConfigPrivate      TweetConfigPrivate;
typedef struct _TweetConfigClass        TweetConfigClass;

struct _TweetConfig
{
  GObject parent_instance;

  TweetConfigPrivate *priv;
};

struct _TweetConfigClass
{
  GObjectClass parent_class;

  void (* changed) (TweetConfig *config);
};

GType                 tweet_config_get_type     (void) G_GNUC_CONST;

TweetConfig *         tweet_config_get_default  (void);

void                  tweet_config_set_username     (TweetConfig *config,
                                                     const gchar *username);
G_CONST_RETURN gchar *tweet_config_get_username     (TweetConfig *config);
void                  tweet_config_set_password     (TweetConfig *config,
                                                     const gchar *password);
G_CONST_RETURN gchar *tweet_config_get_password     (TweetConfig *config);
void                  tweet_config_set_refresh_time (TweetConfig *config,
                                                     guint        seconds);
guint                 tweet_config_get_refresh_time (TweetConfig *config);

gboolean              tweet_config_get_use_gtk_bg   (TweetConfig *config);
void                  tweet_config_set_use_gtk_bg   (TweetConfig *config,
                                                     gboolean     value);

void                  tweet_config_save         (TweetConfig *config);

G_END_DECLS

#endif /* __TWEET_CONFIG_H__ */
