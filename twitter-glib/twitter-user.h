/* twitter-user.h: User representation
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
#ifndef __TWITTER_USER_H__
#define __TWITTER_USER_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <twitter-glib/twitter-common.h>

G_BEGIN_DECLS

/* TwitterUser is declared inside twitter-common.h */

GType                 twitter_user_get_type              (void) G_GNUC_CONST;

TwitterUser *         twitter_user_new                   (void);
TwitterUser *         twitter_user_new_from_data         (const gchar *buffer);

void                  twitter_user_load_from_data        (TwitterUser *user,
                                                          const gchar *buffer);

G_CONST_RETURN gchar *twitter_user_get_name              (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_url               (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_description       (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_location          (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_screen_name       (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_profile_image_url (TwitterUser *user);
guint                 twitter_user_get_id                (TwitterUser *user);
gboolean              twitter_user_get_protected         (TwitterUser *user);

TwitterStatus *       twitter_user_get_status            (TwitterUser *user);

gboolean              twitter_user_get_following         (TwitterUser *user);
guint                 twitter_user_get_friends_count     (TwitterUser *user);
guint                 twitter_user_get_statuses_count    (TwitterUser *user);
guint                 twitter_user_get_followers_count   (TwitterUser *user);
guint                 twitter_user_get_favorites_count   (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_created_at        (TwitterUser *user);
G_CONST_RETURN gchar *twitter_user_get_time_zone         (TwitterUser *user);
gint                  twitter_user_get_utc_offset        (TwitterUser *user);

GdkPixbuf *           twitter_user_get_profile_image     (TwitterUser *user);

G_END_DECLS

#endif /* __TWITTER_USER_H__ */
