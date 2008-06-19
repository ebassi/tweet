/* tweet-status-info.h: Expanded view for a TwitterStatus
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
#ifndef __TWEET_STATUS_INFO_H__
#define __TWEET_STATUS_INFO_H__

#include "tweet-overlay.h"

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_INFO                  (tweet_status_info_get_type ())
#define TWEET_STATUS_INFO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_INFO, TweetStatusInfo))
#define TWEET_IS_STATUS_INFO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_INFO))
#define TWEET_STATUS_INFO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_INFO, TweetStatusInfoClass))
#define TWEET_IS_STATUS_INFO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_INFO))
#define TWEET_STATUS_INFO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_INFO, TweetStatusInfoClass))

typedef struct _TweetStatusInfo         TweetStatusInfo;
typedef struct _TweetStatusInfoClass    TweetStatusInfoClass;

struct _TweetStatusInfo
{
  TweetOverlay parent_instance;

  ClutterActorBox allocation;

  ClutterActor *icon;
  ClutterActor *label;
  ClutterActor *reply_button;
  ClutterActor *star_button;
  ClutterActor *button_tip;

  GRegex *escape_re;

  TwitterStatus *status;
};

struct _TweetStatusInfoClass
{
  TweetOverlayClass parent_class;

  void (* star_clicked)  (TweetStatusInfo *info);
  void (* reply_clicked) (TweetStatusInfo *info);
  void (* icon_clicked)  (TweetStatusInfo *info);
};

GType          tweet_status_info_get_type   (void) G_GNUC_CONST;
ClutterActor * tweet_status_info_new        (TwitterStatus   *status);
void           tweet_status_info_set_status (TweetStatusInfo *info,
                                             TwitterStatus   *status);
TwitterStatus *tweet_status_info_get_status (TweetStatusInfo *info);

G_END_DECLS

#endif /* __TWEET_STATUS_INFO_H__ */
