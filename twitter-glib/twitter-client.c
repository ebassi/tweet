/* twitter-client.h: Wrapper for the Twitter API
 * Copyright (C) 2008  Emmanuele Bassi <ebassi@gnome.org>
 *
 * This file is part of Twitter-GLib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

/**
 * SECTION:twitter-client
 * @short_description: Wrapper for the Twitter API
 *
 * #TwitterClient is the main object wrapping the details of the
 * Twitter web API.
 *
 * In order to use Twitter through #TwitterClient, a new instance
 * should be created using twitter_client_new() or
 * twitter_client_new_for_user(). #TwitterClient handles every
 * operation asynchronously, thus requiring a #GMainLoop running.
 * Every result will emit one of the #TwitterClient signals; in
 * case of error, the #GError parameter of the signals will be
 * set.
 *
 * Authentication is handled automatically by setting the
 * TwitterClient:email and TwitterClient:password properties.
 * These two properties can be set at construction time or by
 * using twitter_client_set_user(). Interactive authentication
 * can be implemented by using the TwitterClient::authenticate
 * signal as well.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gio/gio.h>

#include <libsoup/soup.h>

#include "twitter-api.h"
#include "twitter-client.h"
#include "twitter-common.h"
#include "twitter-enum-types.h"
#include "twitter-marshal.h"
#include "twitter-private.h"
#include "twitter-status.h"
#include "twitter-timeline.h"
#include "twitter-user.h"
#include "twitter-user-list.h"

#define TWITTER_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWITTER_TYPE_CLIENT, TwitterClientPrivate))

struct _TwitterClientPrivate
{
  SoupSession *session_sync;
  SoupSession *session_async;

  gchar *email;
  gchar *password;

  gulong auth_id;

  guint auth_complete : 1;
};

enum
{
  PROP_0,

  PROP_EMAIL,
  PROP_PASSWORD
};

enum
{
  AUTHENTICATE,
  STATUS_RECEIVED,
  USER_RECEIVED,

  LAST_SIGNAL
};

static guint client_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (TwitterClient, twitter_client, G_TYPE_OBJECT);

static void
twitter_client_finalize (GObject *gobject)
{
  TwitterClientPrivate *priv = TWITTER_CLIENT (gobject)->priv;

  soup_session_abort (priv->session_async);
  g_object_unref (priv->session_async);

  soup_session_abort (priv->session_sync);
  g_object_unref (priv->session_sync);

  g_free (priv->email);
  g_free (priv->password);

  G_OBJECT_CLASS (twitter_client_parent_class)->finalize (gobject);
}

static void
twitter_client_set_property (GObject      *gobject,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  TwitterClientPrivate *priv = TWITTER_CLIENT (gobject)->priv;

  switch (prop_id)
    {
    case PROP_EMAIL:
      g_free (priv->email);
      priv->email = g_value_dup_string (value);
      break;

    case PROP_PASSWORD:
      g_free (priv->password);
      priv->password = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
twitter_client_get_property (GObject    *gobject,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  TwitterClientPrivate *priv = TWITTER_CLIENT (gobject)->priv;

  switch (prop_id)
    {
    case PROP_EMAIL:
      g_value_set_string (value, priv->email);
      break;

    case PROP_PASSWORD:
      g_value_set_string (value, priv->password);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
twitter_client_class_init (TwitterClientClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TwitterClientPrivate));

  gobject_class->set_property = twitter_client_set_property;
  gobject_class->get_property = twitter_client_get_property;
  gobject_class->finalize = twitter_client_finalize;

  g_object_class_install_property (gobject_class,
                                   PROP_EMAIL,
                                   g_param_spec_string ("email",
                                                        "Email",
                                                        "The email of the user, for authentication purposes",
                                                        NULL,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_PASSWORD,
                                   g_param_spec_string ("password",
                                                        "Password",
                                                        "The password of the user, for authentication purposes",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  client_signals[AUTHENTICATE] =
    g_signal_new (I_("authenticate"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                  G_STRUCT_OFFSET (TwitterClientClass, authenticate),
                  NULL, NULL,
                  _twitter_marshal_BOOLEAN__ENUM,
                  G_TYPE_BOOLEAN, 1,
                  TWITTER_TYPE_AUTH_STATE);
  client_signals[USER_RECEIVED] =
    g_signal_new (I_("user-received"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterClientClass, user_received),
                  NULL, NULL,
                  _twitter_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  TWITTER_TYPE_USER,
                  G_TYPE_POINTER);
  client_signals[STATUS_RECEIVED] =
    g_signal_new (I_("status-received"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterClientClass, status_received),
                  NULL, NULL,
                  _twitter_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  TWITTER_TYPE_STATUS,
                  G_TYPE_POINTER);
}

static void
twitter_client_init (TwitterClient *client)
{
  TwitterClientPrivate *priv;

  client->priv = priv = TWITTER_CLIENT_GET_PRIVATE (client);

  priv->session_sync =
    soup_session_sync_new_with_options ("user-agent", "Twitter-GLib/" VERSION,
                                        NULL);
  priv->session_async =
    soup_session_async_new_with_options ("user-agent", "Twitter-GLib/" VERSION,
                                         NULL);
}

typedef enum {
  PUBLIC_TIMELINE,
  FRIENDS_TIMELINE,
  USER_TIMELINE,
  STATUS_SHOW,
  UPDATE,
  REPLIES,
  DESTROY,
  FRIENDS,
  FOLLOWERS,
  FEATURED,
  VERIFY_CREDENTIALS,
  END_SESSION,

  N_CLIENT_ACTIONS
} ClientAction;

static const gchar *action_names[N_CLIENT_ACTIONS] = {
  "public-timeline",
  "friends-timeline",
  "user-timeline",
  "status-show",
  "update",
  "replies",
  "destroy",
  "friends",
  "followers",
  "verify-credentials",
  "end-session"
};

typedef struct {
  ClientAction action;
  TwitterClient *client;
  TwitterTimeline *timeline;
} GetTimelineClosure;

typedef struct {
  ClientAction action;
  TwitterClient *client;
  TwitterStatus *status;
} GetStatusClosure;

typedef struct {
  ClientAction action;
  TwitterClient *client;
  TwitterUserList *user_list;
} GetUserListClosure;

typedef struct {
  ClientAction action;
  TwitterClient *client;
  TwitterUser *user;
} GetUserClosure;

static void
twitter_client_auth (SoupSession *session,
                     SoupMessage *msg,
                     SoupAuth    *auth,
                     gboolean     retrying,
                     gpointer     user_data)
{
  TwitterClient *client = user_data;
  TwitterClientPrivate *priv = client->priv;
  gboolean retval = FALSE;

  if (!retrying)
    {
      g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                     TWITTER_AUTH_NEGOTIATING, &retval);

      soup_auth_authenticate (auth, priv->email, priv->password);

      priv->auth_complete = TRUE;
    }
  else
    {
      g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                     TWITTER_AUTH_RETRY, &retval);

      if (G_LIKELY (retval))
        {
          soup_auth_authenticate (auth, priv->email, priv->password);
          priv->auth_complete = TRUE;
        }
      else
        {
          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);
          priv->auth_complete = FALSE;
        }
    }
}

static void
twitter_client_queue_message (TwitterClient       *client,
                              SoupMessage         *msg,
                              gboolean             requires_auth,
                              SoupSessionCallback  callback,
                              gpointer             data)
{
  TwitterClientPrivate *priv = client->priv;

  if (requires_auth && !priv->auth_id)
    priv->auth_id = g_signal_connect (priv->session_async, "authenticate",
                                      G_CALLBACK (twitter_client_auth),
                                      client);

  soup_session_queue_message (priv->session_async, msg,
                              callback,
                              data);
}

TwitterClient *
twitter_client_new (void)
{
  return g_object_new (TWITTER_TYPE_CLIENT, NULL);
}

TwitterClient *
twitter_client_new_for_user (const gchar *email,
                             const gchar *password)
{
  return g_object_new (TWITTER_TYPE_CLIENT,
                       "email", email,
                       "password", password,
                       NULL);
}

void
twitter_client_set_user (TwitterClient *client,
                         const gchar   *email,
                         const gchar   *password)
{
  TwitterClientPrivate *priv;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (email != NULL);
  g_return_if_fail (password != NULL);

  priv = client->priv;

  g_free (priv->email);
  g_free (priv->password);

  priv->email = g_strdup (email);
  priv->password = g_strdup (password);

  priv->auth_complete = FALSE;

  g_object_notify (G_OBJECT (client), "email");
  g_object_notify (G_OBJECT (client), "password");
}

void
twitter_client_get_user (TwitterClient  *client,
                         gchar         **email,
                         gchar         **password)
{
  g_return_if_fail (TWITTER_IS_CLIENT (client));

  if (email)
    *email = g_strdup (client->priv->email);
  if (password)
    *password = g_strdup (client->priv->password);
}

typedef struct {
  TwitterClient *client;
  GList *users;
  guint n_users;
  guint current_user;
} EmitUserClosure;

static gboolean
do_emit_user_received (gpointer data)
{
  return FALSE;
}

static void
cleanup_emit_user_received (gpointer data)
{

}

static void
emit_user_received (TwitterClient *client,
                    TwitterUserList *list)
{

}

static void
get_user_cb (SoupSession *session,
             SoupMessage *msg,
             gpointer     user_data)
{
  GetStatusClosure *closure = user_data;
  TwitterClient *client = closure->client; 

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     closure->status, error);
    }
  else
    {
      gchar *buffer;

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);
      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_status_load_from_data (closure->status, buffer);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     closure->status, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->client);
  g_object_unref (closure->status);

  g_free (closure);
}

void
twitter_client_verify_user (TwitterClient *client)
{
#if 0
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_verify_credentials ();

  clos = g_new0 (VerifyClosure, 1);
  clos->action = VERIFY_CREDENTIALS;
  clos->client = g_object_ref (client);
  clos->reply = NULL;

  twitter_client_queue_message (client, msg, TRUE,
                                verify_cb,
                                clos);
#endif
}

static void
end_session_cb (SoupSession *session,
                SoupMessage *message,
                gpointer     user_data)
{
  TwitterClient *client = user_data;

  client->priv->auth_complete = FALSE;
}

void
twitter_client_end_session (TwitterClient *client)
{
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_end_session ();

  twitter_client_queue_message (client, msg, FALSE,
                                end_session_cb,
                                client);
}

typedef struct {
  TwitterClient *client;
  TwitterTimeline *timeline;
  guint n_status;
  guint current_status;
} EmitStatusClosure;

static gboolean
do_emit_status_received (gpointer data)
{
  EmitStatusClosure *closure = data;
  TwitterStatus *status;

  status = twitter_timeline_get_pos (closure->timeline,
                                     closure->current_status);
  if (!status)
    return FALSE;

  g_signal_emit (closure->client, client_signals[STATUS_RECEIVED], 0,
                 status, NULL);

  closure->current_status += 1;

  if (closure->current_status == closure->n_status)
    return FALSE;

  return TRUE;
}

static void
cleanup_emit_status_received (gpointer data)
{
  EmitStatusClosure *closure = data;

  g_object_unref (closure->client);
  g_object_unref (closure->timeline);

  g_free (closure);
}

static void
emit_status_received (TwitterClient   *client,
                      TwitterTimeline *timeline)
{
  EmitStatusClosure *closure;
  guint count;

  count = twitter_timeline_get_count (timeline);

  closure = g_new (EmitStatusClosure, 1);
  closure->client = g_object_ref (client);
  closure->timeline = g_object_ref (timeline);
  closure->n_status = count;
  closure->current_status = 0;

  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                   do_emit_status_received,
                   closure,
                   cleanup_emit_status_received);
}

static void
get_timeline_cb (SoupSession *session,
                 SoupMessage *msg,
                 gpointer     user_data)
{
  GetTimelineClosure *closure = user_data;
  TwitterClient *client = closure->client;
  guint status;

  status = msg->status_code;
  if (!SOUP_STATUS_IS_SUCCESSFUL (status))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (status),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     NULL, error);
    }
  else
    {
      gchar *buffer;

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_timeline_load_from_data (closure->timeline, buffer);

      emit_status_received (closure->client, closure->timeline);

      g_free (buffer);
    }

  g_object_unref (closure->timeline);
  g_object_unref (closure->client);

  g_free (closure);
}

void
twitter_client_get_public_timeline (TwitterClient *client,
                                    guint          since_id)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_public_timeline (since_id);

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = PUBLIC_TIMELINE;
  clos->client = g_object_ref (client);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, FALSE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_friends_timeline (TwitterClient *client,
                                     const gchar   *friend_,
                                     const gchar   *since_date)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_friends_timeline (friend_, since_date);

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = FRIENDS_TIMELINE;
  clos->client = g_object_ref (client);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_user_timeline (TwitterClient *client,
                                  const gchar   *user,
                                  guint          count,
                                  const gchar   *since_date)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_user_timeline (user, count, since_date);

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = USER_TIMELINE;
  clos->client = g_object_ref (client);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_replies (TwitterClient *client)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_replies ();

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = REPLIES;
  clos->client = g_object_ref (client);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_favorites (TwitterClient *client,
                              const gchar   *user)
{
#if 0
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_favorites (user, -1);

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = FAVORITES;
  clos->client = g_object_ref (client);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
#endif
}

static void
get_status_cb (SoupSession *session,
               SoupMessage *msg,
               gpointer     user_data)
{
  GetStatusClosure *closure = user_data;
  TwitterClient *client = closure->client;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     NULL, error);
    }
  else
    {
      gchar *buffer;

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_status_load_from_data (closure->status, buffer);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     closure->status, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->client);
  g_object_unref (closure->status);

  g_free (closure);
}

void
twitter_client_get_status (TwitterClient *client,
                           guint          status_id)
{
  GetStatusClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (status_id > 0);

  msg = twitter_api_status_show (status_id);

  clos = g_new0 (GetStatusClosure, 1);
  clos->action = STATUS_SHOW;
  clos->client = g_object_ref (client);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, FALSE,
                                get_status_cb,
                                clos);
}

void
twitter_client_add_status (TwitterClient *client,
                           const gchar   *text)
{
  GetStatusClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (text != NULL);

  msg = twitter_api_update (text);

  clos = g_new0 (GetStatusClosure, 1);
  clos->action = UPDATE;
  clos->client = g_object_ref (client);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_status_cb,
                                clos);
}

void
twitter_client_remove_status (TwitterClient *client,
                              guint          status_id)
{
  GetStatusClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (status_id > 0);

  msg = twitter_api_destroy (status_id);

  clos = g_new0 (GetStatusClosure, 1);
  clos->action = DESTROY;
  clos->client = g_object_ref (client);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_status_cb,
                                clos);
}

void
twitter_client_add_friend (TwitterClient *client,
                           const gchar   *user)
{

}

void
twitter_client_remove_friend (TwitterClient *client,
                              const gchar   *user)
{

}

void
twitter_client_follow_user (TwitterClient *client,
                            const gchar   *user)
{

}

void
twitter_client_leave_user (TwitterClient  *client,
                           const gchar    *user)
{

}

void
twitter_client_add_favorite (TwitterClient  *client,
                             guint           status_id)
{

}

void
twitter_client_remove_favorite (TwitterClient  *client,
                                guint           status_id)
{

}
