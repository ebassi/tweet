/* tweet-status-model.h: Model for the status view
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
#ifndef __TWEET_STATUS_MODEL_H__
#define __TWEET_STATUS_MODEL_H__

#include <clutter/clutter-model.h>
#include <twitter-glib/twitter-glib.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_MODEL                 (tweet_status_model_get_type ())
#define TWEET_STATUS_MODEL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_MODEL, TweetStatusModel))
#define TWEET_IS_STATUS_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_MODEL))
#define TWEET_STATUS_MODEL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_MODEL, TweetStatusModelClass))
#define TWEET_IS_STATUS_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_MODEL))
#define TWEET_STATUS_MODEL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_MODEL, TweetStatusModelClass))

typedef struct _TweetStatusModel              TweetStatusModel;
typedef struct _TweetStatusModelPrivate       TweetStatusModelPrivate;
typedef struct _TweetStatusModelClass         TweetStatusModelClass;

struct _TweetStatusModel
{
  ClutterModel parent_instance;

  TweetStatusModelPrivate *priv;
};

struct _TweetStatusModelClass
{
  ClutterModelClass parent_class;
};

GType tweet_status_model_get_type (void);

ClutterModel * tweet_status_model_new            (void);
gboolean       tweet_status_model_append_status  (TweetStatusModel *model,
                                                  TwitterStatus    *status);
gboolean       tweet_status_model_prepend_status (TweetStatusModel *model,
                                                  TwitterStatus    *status);

TwitterStatus *tweet_status_model_get_status     (TweetStatusModel *model,
                                                  ClutterModelIter *iter);

G_END_DECLS

#endif /* __TWEET_STATUS_MODEL_H__ */
