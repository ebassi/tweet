/* twitter-api.h: Wrappers for the Twitter RESTful API
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
#ifndef __TWITTER_API_H__
#define __TWITTER_API_H__

#include <glib.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

SoupMessage *twitter_api_public_timeline    (gint         since_id);
SoupMessage *twitter_api_friends_timeline   (const gchar *user,
                                             gint64       since);
SoupMessage *twitter_api_user_timeline      (const gchar *user,
                                             guint        count,
                                             gint64       since);
SoupMessage *twitter_api_status_show        (guint        status_id);
SoupMessage *twitter_api_update             (const gchar *text);
SoupMessage *twitter_api_replies            (void);
SoupMessage *twitter_api_destroy            (guint        status_id);
SoupMessage *twitter_api_friends            (const gchar *user,
                                             gint         page,
                                             gboolean     lite);
SoupMessage *twitter_api_featured           (void);
SoupMessage *twitter_api_user_show          (const gchar *user,
                                             const gchar *email);
SoupMessage *twitter_api_verify_credentials (void);
SoupMessage *twitter_api_end_session        (void);
SoupMessage *twitter_api_followers          (gint         page,
                                             gboolean     lite);
SoupMessage *twitter_api_create_friend      (const gchar *user);
SoupMessage *twitter_api_destroy_friend     (const gchar *user);
SoupMessage *twitter_api_favorites          (const gchar *user,
                                             gint         page);
SoupMessage *twitter_api_create_favorite    (guint        status_id);
SoupMessage *twitter_api_destroy_favorite   (guint        status_id);
SoupMessage *twitter_api_follow             (const gchar *user);
SoupMessage *twitter_api_leave              (const gchar *user);
SoupMessage *twitter_api_archive            (gint         page);

G_END_DECLS

#endif /* __TWITTER_API_H__ */
