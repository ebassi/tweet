/* twitter-user-list.h: Collection of users
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
#ifndef __TWITTER_USER_LIST_H__
#define __TWITTER_USER_LIST_H__

#include <glib-object.h>
#include <twitter-glib/twitter-status.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_USER_LIST            (twitter_user_list_get_type ())
#define TWITTER_USER_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_USER_LIST, TwitterUserList))
#define TWITTER_IS_USER_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_USER_LIST))
#define TWITTER_USER_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_USER_LIST, TwitterUserListClass))
#define TWITTER_IS_USER_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_USER_LIST))
#define TWITTER_USER_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_USER_LIST, TwitterUserListClass))

typedef struct _TwitterUserList         TwitterUserList;
typedef struct _TwitterUserListPrivate  TwitterUserListPrivate;
typedef struct _TwitterUserListClass    TwitterUserListClass;

struct _TwitterUserList
{
  GObject parent_instance;

  TwitterUserListPrivate *priv;
};

struct _TwitterUserListClass
{
  GObjectClass parent_class;
};

GType            twitter_user_list_get_type       (void) G_GNUC_CONST;

TwitterUserList *twitter_user_list_new            (void);
TwitterUserList *twitter_user_list_new_from_data  (const gchar     *buffer);

void             twitter_user_list_load_from_data (TwitterUserList *user_list,
                                                   const gchar     *buffer);

guint            twitter_user_list_get_count      (TwitterUserList *user_list);
TwitterUser   *  twitter_user_list_get_id         (TwitterUserList *user_list,
                                                   guint            id);
TwitterUser   *  twitter_user_list_get_pos        (TwitterUserList *user_list,
                                                   gint             index_);
GList *          twitter_user_list_get_all        (TwitterUserList *user_list);

G_END_DECLS

#endif /* __TWITTER_USER_LIST_H__ */
