/* twitter-client.h: Client for Twitter RESTful API
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
#ifndef __TWITTER_CLIENT_H__
#define __TWITTER_CLIENT_H__

#include <glib-object.h>

#include <twitter-glib/twitter-status.h>
#include <twitter-glib/twitter-user.h>

G_BEGIN_DECLS

#define TWITTER_TYPE_CLIENT             (twitter_client_get_type ())
#define TWITTER_CLIENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWITTER_TYPE_CLIENT, TwitterClient))
#define TWITTER_IS_CLIENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWITTER_TYPE_CLIENT))
#define TWITTER_CLIENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TWITTER_TYPE_CLIENT, TwitterClientClass))
#define TWITTER_IS_CLIENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TWITTER_TYPE_CLIENT))
#define TWITTER_CLIENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TWITTER_TYPE_CLIENT, TwitterClientClass))

typedef struct _TwitterClient           TwitterClient;
typedef struct _TwitterClientPrivate    TwitterClientPrivate;
typedef struct _TwitterClientClass      TwitterClientClass;

/**
 * TwitterAuthState:
 * @TWITTER_AUTH_NEGOTIATING: Initial phase of the authentication
 * @TWITTER_AUTH_RETRY: Retry the authentication
 * @TWITTER_AUTH_FAILED: Authentication failed
 * @TWITTER_AUTH_SUCCESS: Authentication succeded
 *
 * Enumeration used to distinguish the states of the authentication process
 * in the #TwitterClient::authenticate signal handlers.
 */
typedef enum {
  TWITTER_AUTH_NEGOTIATING,
  TWITTER_AUTH_RETRY,
  TWITTER_AUTH_FAILED,
  TWITTER_AUTH_SUCCESS
} TwitterAuthState;

/**
 * TwitterClient:
 *
 * #TwitterClient allows to connect to the Twitter web service
 * and operate using the RESTful API provided from any GLib-based
 * application.
 *
 * The #TwitterClient-struct struct contains private data only, and
 * should only be accessed using the functions below.
 */
struct _TwitterClient
{
  /*< private >*/
  GObject parent_instance;

  TwitterClientPrivate *priv;
};

/**
 * TwitterClientClass;
 * @authenticate: class handler for the #TwitterClient::authenticate signal
 * @user_verified: class handler for the #TwitterClient::user-verified signal
 * @session_ended: class handler for the #TwitterClient::session-eneded signal
 * @status_received: class handler for the #TwitterClient::status-received
 *   signal
 * @user_received: class handler for the #TwitterClient::user-received signal
 * @timeline_complete: class handler for the #TwitterClient::timeline_complete
 *   signal
 *
 * Base class for #TwitterClient.
 */
struct _TwitterClientClass
{
  /*< private >*/
  GObjectClass parent_instance;

  /*< public >*/
  gboolean (* authenticate)      (TwitterClient    *client,
                                  TwitterAuthState  state);

  void     (* user_verified)     (TwitterClient    *client,
                                  gboolean          is_verified,
                                  const GError     *error);
  void     (* session_ended)     (TwitterClient    *client,
                                  TwitterUser      *user,
                                  const GError     *error);

  void     (* status_received)   (TwitterClient    *client,
                                  TwitterStatus    *status,
                                  const GError     *error);
  void     (* user_received)     (TwitterClient    *client,
                                  TwitterUser      *user,
                                  const GError     *error);

  void     (* timeline_complete) (TwitterClient    *client);

  /*< private >*/
  /* padding, for future expansion */
  void     (* _twitter_padding1) (void);
  void     (* _twitter_padding2) (void);
  void     (* _twitter_padding3) (void);
  void     (* _twitter_padding4) (void);
  void     (* _twitter_padding5) (void);
  void     (* _twitter_padding6) (void);
  void     (* _twitter_padding7) (void);
  void     (* _twitter_padding8) (void);
};

GType          twitter_client_get_type             (void) G_GNUC_CONST;

TwitterClient *twitter_client_new                  (void);
TwitterClient *twitter_client_new_for_user         (const gchar    *email,
                                                    const gchar    *password);

void           twitter_client_set_user             (TwitterClient  *client,
                                                    const gchar    *email,
                                                    const gchar    *password);
void           twitter_client_get_user             (TwitterClient  *client,
                                                    gchar         **email,
                                                    gchar         **password);
void           twitter_client_verify_user          (TwitterClient  *client);
void           twitter_client_end_session          (TwitterClient  *client);
void           twitter_client_show_user_from_id    (TwitterClient  *client,
                                                    const gchar    *user);
void           twitter_client_show_user_from_email (TwitterClient  *client,
                                                    const gchar    *email);

void           twitter_client_get_public_timeline  (TwitterClient  *client,
                                                    guint           since_id);
void           twitter_client_get_friends_timeline (TwitterClient  *client,
                                                    const gchar    *friend_,
                                                    gint64          since_date);
void           twitter_client_get_user_timeline    (TwitterClient  *client,
                                                    const gchar    *user,
                                                    guint           count,
                                                    gint64          since_date);
void           twitter_client_get_replies          (TwitterClient  *client);
void           twitter_client_get_favorites        (TwitterClient  *client,
                                                    const gchar    *user,
                                                    gint            page);
void           twitter_client_get_archive          (TwitterClient  *client,
                                                    gint            page);
void           twitter_client_get_friends          (TwitterClient  *client,
                                                    const gchar    *user,
                                                    gint            page,
                                                    gboolean        omit_status);
void           twitter_client_get_followers        (TwitterClient  *client,
                                                    gint            page,
                                                    gboolean        omit_status);

void           twitter_client_get_status           (TwitterClient  *client,
                                                    guint           status_id);
void           twitter_client_add_status           (TwitterClient  *client,
                                                    const gchar    *text);
void           twitter_client_remove_status        (TwitterClient  *client,
                                                    guint           status_id);

void           twitter_client_add_friend           (TwitterClient  *client,
                                                    const gchar    *user);
void           twitter_client_remove_friend        (TwitterClient  *client,
                                                    const gchar    *user);

void           twitter_client_follow_user          (TwitterClient  *client,
                                                    const gchar    *user);
void           twitter_client_leave_user           (TwitterClient  *client,
                                                    const gchar    *user);

void           twitter_client_add_favorite         (TwitterClient  *client,
                                                    guint           status_id);
void           twitter_client_remove_favorite      (TwitterClient  *client,
                                                    guint           status_id);

G_END_DECLS

#endif /* __TWITTER_CLIENT_H__ */
