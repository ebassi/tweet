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

/**
 * SECTION:twitter-client
 * @short_description: Wrapper for the Twitter API
 *
 * #TwitterClient is the main object wrapping the details of the
 * Twitter RESTful API.
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
 * #TwitterClient:email and #TwitterClient:password properties.
 * These two properties can be set at construction time or by
 * using twitter_client_set_user(). Interactive authentication
 * can be implemented by using the #TwitterClient::authenticate
 * signal as well: when the signal passes the #TwitterAuthState value
 * or %TWITTER_AUTH_NEGOTIATING the handler should set the user
 * credentials with twitter_client_set_user(); in case of error,
 * the ::authenticate signal will use %TWITTER_AUTH_RETRY until the
 * authentication succeeds or the signal handler returns %FALSE,
 * in which case the %TWITTER_AUTH_FAILED state will be used.
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

#ifndef G_WARN_NOT_IMPLEMENTED
#define G_WARN_NOT_IMPLEMENTED  (g_warning (G_STRLOC ": This function has not been implemented yet"))
#endif

#define TWITTER_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWITTER_TYPE_CLIENT, TwitterClientPrivate))

struct _TwitterClientPrivate
{
  SoupSession *session_async;

  gchar *user_agent;

  gchar *email;
  gchar *password;

  gulong auth_id;

  guint auth_complete : 1;
};

enum
{
  PROP_0,

  PROP_EMAIL,
  PROP_PASSWORD,
  PROP_USER_AGENT
};

enum
{
  AUTHENTICATE,
  STATUS_RECEIVED,
  USER_RECEIVED,
  TIMELINE_COMPLETE,
  USER_VERIFIED,

  LAST_SIGNAL
};

static guint client_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (TwitterClient, twitter_client, G_TYPE_OBJECT);

#ifdef TWEET_ENABLE_DEBUG
static inline void
twitter_debug (const gchar *action,
               const gchar *buffer)
{
  if (g_getenv ("TWITTER_GLIB_DEBUG") != NULL)
    g_print ("[DEBUG]:%s: %s\n", action, buffer);
}
#else
# define twitter_debug(a,b)
#endif /* TWEET_ENABLE_DEBUG */

static void
twitter_client_finalize (GObject *gobject)
{
  TwitterClientPrivate *priv = TWITTER_CLIENT (gobject)->priv;

  soup_session_abort (priv->session_async);
  g_object_unref (priv->session_async);

  g_free (priv->user_agent);
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

    case PROP_USER_AGENT:
      g_free (priv->user_agent);
      priv->user_agent = g_value_dup_string (value);
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

    case PROP_USER_AGENT:
      g_value_set_string (value, priv->user_agent);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
twitter_client_constructed (GObject *gobject)
{
  TwitterClientPrivate *priv = TWITTER_CLIENT (gobject)->priv;
  gchar *user_agent;

  if (!priv->user_agent)
    user_agent = g_strdup ("Twitter-GLib/" VERSION);
  else
    user_agent = g_strdup (priv->user_agent);

  priv->session_async =
    soup_session_async_new_with_options ("user-agent", user_agent, NULL);

  g_free (user_agent);
}

static void
twitter_client_class_init (TwitterClientClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TwitterClientPrivate));

  gobject_class->constructed = twitter_client_constructed;
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
  g_object_class_install_property (gobject_class,
                                   PROP_USER_AGENT,
                                   g_param_spec_string ("user-agent",
                                                        "User Agent",
                                                        "The client name to be used when connecting",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  /**
   * TwitterClient::authenticate:
   * @client: the #TwitterClient that received the signal
   * @state: the state of the authentication process
   *
   * Handles the authentication of the user onto the Twitter services.
   *
   * The authentication can be a multi-state process. If the user's
   * credentials were not set before issuing a command that requires
   * authentication, the ::authenticate signal will be emitted with the
   * %TWITTER_AUTH_NEGOTIATING state. In this case, the credentials must
   * be set and the handler must return %TRUE.
   *
   * |[
   *   if (state == TWITTER_AUTH_NEGOTIATING)
   *     {
   *       twitter_client_set_user (client, email, password);
   *       return TRUE;
   *     }
   * ]|
   *
   * In case of failed authentication, the %TWITTER_AUTH_RETRY state
   * will be used until the handler sets the correct credentials and
   * returns %TRUE or aborts the authentication process and returns
   * %FALSE; in the latter case, the signal will be emitted one last
   * time with the %TWITTER_AUTH_FAILED state.
   *
   * If the authentication was successful, the signal will be emitted
   * with the %TWITTER_AUTH_SUCCESS state.
   *
   * Return value: %TRUE if the user credentials were correctly set
   */
  client_signals[AUTHENTICATE] =
    g_signal_new (I_("authenticate"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                  G_STRUCT_OFFSET (TwitterClientClass, authenticate),
                  NULL, NULL,
                  _twitter_marshal_BOOLEAN__ENUM,
                  G_TYPE_BOOLEAN, 1,
                  TWITTER_TYPE_AUTH_STATE);
  client_signals[USER_VERIFIED] =
    g_signal_new (I_("user-verified"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterClientClass, user_verified),
                  NULL, NULL,
                  _twitter_marshal_VOID__BOOLEAN_POINTER,
                  G_TYPE_NONE, 2,
                  G_TYPE_BOOLEAN,
                  G_TYPE_POINTER);
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
  client_signals[TIMELINE_COMPLETE] =
    g_signal_new (I_("timeline-complete"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterClientClass, timeline_complete),
                  NULL, NULL,
                  _twitter_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
twitter_client_init (TwitterClient *client)
{
  TwitterClientPrivate *priv;

  client->priv = priv = TWITTER_CLIENT_GET_PRIVATE (client);

  priv->auth_id = 0;
}

typedef enum {
  PUBLIC_TIMELINE,
  FRIENDS_TIMELINE,
  USER_TIMELINE,
  STATUS_SHOW,
  STATUS_UPDATE,
  STATUS_REPLIES,
  STATUS_DESTROY,
  FRIENDS,
  FOLLOWERS,
  FEATURED,
  USER_SHOW,
  VERIFY_CREDENTIALS,
  END_SESSION,
  ARCHIVE,
  FRIEND_CREATE,
  FRIEND_DESTROY,
  FAVORITE_CREATE,
  FAVORITE_DESTROY,
  FAVORITES,
  NOTIFICATION_FOLLOW,
  NOTIFICATION_LEAVE,

  N_CLIENT_ACTIONS
} ClientAction;

#ifdef TWEET_ENABLE_DEBUG
/* XXX - keep in sync with the enumeration above */
static const gchar *action_names[N_CLIENT_ACTIONS] = {
  "statuses/public_timeline",
  "statuses/friends_timeline",
  "statuses/user_timeline",
  "statuses/show",
  "statuses/update",
  "statuses/replies",
  "statuses/destroy",
  "statuses/friends",
  "statuses/followers",
  "statuses/featured",
  "users/show",
  "account/verify_credentials",
  "account/end_session",
  "account/archive",
  "friendship/create",
  "friendship/destroy",
  "favorites/create",
  "favorites/destroy",
  "favorites",
  "notifications/follow",
  "notifications/leave"
};
#endif /* TWEET_ENABLE_DEBUG */

typedef struct {
  ClientAction action;
  TwitterClient *client;
  guint requires_auth : 1;
} ClientClosure;

#define closure_set_action(c,v)         (((ClientClosure *) (c))->action) = (v)
#define closure_get_action(c)           (((ClientClosure *) (c))->action)
#define closure_set_client(c,v)         (((ClientClosure *) (c))->client) = (v)
#define closure_get_client(c)           (((ClientClosure *) (c))->client)
#define closure_set_requires_auth(c,v)  (((ClientClosure *) (c))->requires_auth) = (v)
#define closure_get_requires_auth(c)    (((ClientClosure *) (c))->requires_auth)

#ifdef TWEET_ENABLE_DEBUG
#define closure_get_action_name(c)      (action_names[(((ClientClosure *) (c))->action)])
#else
#define closure_get_action_name(c)      '\0'
#endif

typedef struct {
  ClientClosure closure;
  TwitterTimeline *timeline;
} GetTimelineClosure;

typedef struct {
  ClientClosure closure;
  TwitterStatus *status;
} GetStatusClosure;

typedef struct {
  ClientClosure closure;
  TwitterUserList *user_list;
} GetUserListClosure;

typedef struct {
  ClientClosure closure;
  TwitterUser *user;
} GetUserClosure;

typedef struct {
  ClientClosure closure;
} VerifyClosure;

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

      if (!priv->email || !priv->password)
        {
          priv->auth_complete = FALSE;
          return;
        }

      soup_auth_authenticate (auth, priv->email, priv->password);
    }
  else
    {
      g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                     TWITTER_AUTH_RETRY, &retval);

      if (G_LIKELY (retval))
        soup_auth_authenticate (auth, priv->email, priv->password);
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
  TwitterUserList *user_list;
  guint n_users;
  guint current_user;
} EmitUserClosure;

static gboolean
do_emit_user_received (gpointer data)
{
  EmitUserClosure *closure = data;
  TwitterUser *user;

  user = twitter_user_list_get_pos (closure->user_list,
                                    closure->current_user);
  if (!user)
    return FALSE;

  g_signal_emit (closure->client, client_signals[USER_RECEIVED], 0,
                 user, NULL);

  closure->current_user += 1;

  if (closure->current_user == closure->n_users)
    return FALSE;

  return TRUE;
}

static void
cleanup_emit_user_received (gpointer data)
{
  EmitUserClosure *closure = data;

  g_object_unref (closure->client);
  g_object_unref (closure->user_list);

  g_free (closure);
}

static void
emit_user_received (TwitterClient   *client,
                    TwitterUserList *user_list)
{
  EmitUserClosure *closure;
  guint count;

  count = twitter_user_list_get_count (user_list);

  closure = g_new (EmitUserClosure, 1);
  closure->client = g_object_ref (client);
  closure->user_list = g_object_ref (user_list);
  closure->n_users = count;
  closure->current_user = 0;

  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE + 50,
                   do_emit_user_received,
                   closure,
                   cleanup_emit_user_received);
}

static void
get_status_cb (SoupSession *session,
               SoupMessage *msg,
               gpointer     user_data)
{
  GetStatusClosure *closure = user_data;
  gboolean requires_auth = closure_get_requires_auth (closure);
  TwitterClient *client = closure_get_client (closure);
  TwitterClientPrivate *priv = client->priv;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      if (msg->status_code == SOUP_STATUS_UNAUTHORIZED)
        {
          gboolean retval = FALSE;

          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);

          priv->auth_complete = FALSE;
        }

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     closure->status, error);

      g_error_free (error);
    }
  else
    {
      gboolean retval = FALSE;
      gchar *buffer;

      if (requires_auth && !priv->auth_complete)
        {
          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_SUCCESS, &retval);
          priv->auth_complete = TRUE;
        }

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      twitter_debug (closure_get_action_name (closure), buffer);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_status_load_from_data (closure->status, buffer);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     closure->status, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->status);
  g_object_unref (client);

  g_free (closure);
}

void
verify_cb (SoupSession *session,
           SoupMessage *msg,
           gpointer     user_data)
{
  VerifyClosure *closure = user_data;
  TwitterClient *client = closure_get_client (closure);

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[USER_VERIFIED], 0,
                     FALSE, error);

      g_error_free (error);
    }
  else
    {
      gboolean is_verified;

      is_verified = (msg->status_code == 200);

      g_signal_emit (client, client_signals[USER_VERIFIED], 0,
                     is_verified, NULL);
    }

  g_object_unref (client);

  g_free (closure);
}

void
twitter_client_verify_user (TwitterClient *client)
{
  VerifyClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_verify_credentials ();

  clos = g_new0 (VerifyClosure, 1);
  closure_set_action (clos, VERIFY_CREDENTIALS);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);

  twitter_client_queue_message (client, msg, TRUE,
                                verify_cb,
                                clos);
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
    {
      g_signal_emit (closure->client, client_signals[TIMELINE_COMPLETE], 0);
      return FALSE;
    }

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

  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE + 50,
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
  gboolean requires_auth = closure_get_requires_auth (closure);
  TwitterClient *client = closure_get_client (closure);
  TwitterClientPrivate *priv = client->priv;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      if (msg->status_code == SOUP_STATUS_UNAUTHORIZED)
        {
          gboolean retval = FALSE;

          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);

          priv->auth_complete = FALSE;
        }

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[STATUS_RECEIVED], 0,
                     NULL, error);

      g_error_free (error);
    }
  else
    {
      gboolean retval = FALSE;
      gchar *buffer;

      if (requires_auth && !priv->auth_complete)
        {
          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_SUCCESS, &retval);
          priv->auth_complete = TRUE;
        }

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_timeline_load_from_data (closure->timeline, buffer);

      emit_status_received (client, closure->timeline);

      g_free (buffer);
    }

  g_object_unref (closure->timeline);
  g_object_unref (client);

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
  closure_set_action (clos, PUBLIC_TIMELINE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, FALSE);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, FALSE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_friends_timeline (TwitterClient *client,
                                     const gchar   *friend_,
                                     gint64         since_date)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_friends_timeline (friend_, since_date);

  clos = g_new0 (GetTimelineClosure, 1);
  closure_set_action (clos, FRIENDS_TIMELINE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_user_timeline (TwitterClient *client,
                                  const gchar   *user,
                                  guint          count,
                                  gint64         since_date)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_user_timeline (user, count, since_date);

  clos = g_new0 (GetTimelineClosure, 1);
  closure_set_action (clos, USER_TIMELINE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
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
  closure_set_action (clos, STATUS_REPLIES);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_favorites (TwitterClient *client,
                              const gchar   *user,
                              gint           page)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_favorites (user, page);

  clos = g_new0 (GetTimelineClosure, 1);
  closure_set_action (clos, FAVORITES);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

void
twitter_client_get_archive (TwitterClient *client,
                            gint           page)
{
  GetTimelineClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_archive (page);

  clos = g_new0 (GetTimelineClosure, 1);
  closure_set_action (clos, ARCHIVE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->timeline = twitter_timeline_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_timeline_cb,
                                clos);
}

static void
get_user_cb (SoupSession *session,
             SoupMessage *msg,
             gpointer     user_data)
{
  GetUserClosure *closure = user_data;
  gboolean requires_auth = closure_get_requires_auth (closure);
  TwitterClient *client = closure_get_client (closure);
  TwitterClientPrivate *priv = client->priv;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      if (msg->status_code == SOUP_STATUS_UNAUTHORIZED)
        {
          gboolean retval = FALSE;

          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);

          priv->auth_complete = FALSE;
        }

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[USER_RECEIVED], 0,
                     NULL, error);

      g_error_free (error);
    }
  else
    {
      gboolean retval = FALSE;
      gchar *buffer;

      if (requires_auth && !priv->auth_complete)
        {
          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_SUCCESS, &retval);
          priv->auth_complete = TRUE;
        }

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      twitter_debug (closure_get_action_name (closure), buffer);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_user_load_from_data (closure->user, buffer);

      g_signal_emit (client, client_signals[USER_RECEIVED], 0,
                     closure->user, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->user);
  g_object_unref (client);

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
  closure_set_action (clos, STATUS_SHOW);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, FALSE);
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
  closure_set_action (clos, STATUS_UPDATE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
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
  closure_set_action (clos, STATUS_DESTROY);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_status_cb,
                                clos);
}

static void
get_user_list_cb (SoupSession *session,
                  SoupMessage *msg,
                  gpointer     user_data)
{
  GetUserListClosure *closure = user_data;
  gboolean requires_auth = closure_get_requires_auth (closure);
  TwitterClient *client = closure_get_client (closure);
  TwitterClientPrivate *priv = client->priv;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      if (msg->status_code == SOUP_STATUS_UNAUTHORIZED)
        {
          gboolean retval = FALSE;

          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);

          priv->auth_complete = FALSE;
        }

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (client, client_signals[USER_RECEIVED], 0,
                     NULL, error);

      g_error_free (error);
    }
  else
    {
      gboolean retval = FALSE;
      gchar *buffer;

      if (requires_auth && !priv->auth_complete)
        {
          g_signal_emit (client, client_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_SUCCESS, &retval);
          priv->auth_complete = TRUE;
        }

      buffer = g_strndup (msg->response_body->data,
                          msg->response_body->length);

      if (G_UNLIKELY (!buffer))
        g_warning ("No data received");
      else
        twitter_user_list_load_from_data (closure->user_list, buffer);

      emit_user_received (client, closure->user_list);

      g_free (buffer);
    }

  g_object_unref (closure->user_list);
  g_object_unref (client);

  g_free (closure);
}

void
twitter_client_add_friend (TwitterClient *client,
                           const gchar   *user)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (user != NULL);

  msg = twitter_api_create_friend (user);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, FRIEND_CREATE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}

void
twitter_client_remove_friend (TwitterClient *client,
                              const gchar   *user)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (user != NULL);

  msg = twitter_api_destroy_friend (user);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, FRIEND_DESTROY);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}

void
twitter_client_follow_user (TwitterClient *client,
                            const gchar   *user)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (user != NULL);

  msg = twitter_api_follow (user);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, NOTIFICATION_FOLLOW);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}

void
twitter_client_leave_user (TwitterClient  *client,
                           const gchar    *user)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (user != NULL);

  msg = twitter_api_leave (user);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, NOTIFICATION_LEAVE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}

void
twitter_client_add_favorite (TwitterClient  *client,
                             guint           status_id)
{
  GetStatusClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (status_id > 0);

  msg = twitter_api_create_favorite (status_id);

  clos = g_new0 (GetStatusClosure, 1);
  closure_set_action (clos, FAVORITE_CREATE);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_status_cb,
                                clos);
}

void
twitter_client_remove_favorite (TwitterClient  *client,
                                guint           status_id)
{
  GetStatusClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (status_id > 0);

  msg = twitter_api_destroy_favorite (status_id);

  clos = g_new0 (GetStatusClosure, 1);
  closure_set_action (clos, FAVORITE_DESTROY);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (close, TRUE);
  clos->status = twitter_status_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_status_cb,
                                clos);
}

void
twitter_client_get_friends (TwitterClient *client,
                            const gchar   *user,
                            gint           page,
                            gboolean       omit_status)
{
  GetUserListClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_friends (user, page, omit_status);

  clos = g_new0 (GetUserListClosure, 1);
  closure_set_action (clos, FRIENDS);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (close, TRUE);
  clos->user_list = twitter_user_list_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_list_cb,
                                clos);
}

void
twitter_client_get_followers (TwitterClient *client,
                              gint           page,
                              gboolean       omit_status)
{
  GetUserListClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));

  msg = twitter_api_followers (page, omit_status);

  clos = g_new0 (GetUserListClosure, 1);
  closure_set_action (clos, FOLLOWERS);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (close, TRUE);
  clos->user_list = twitter_user_list_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_list_cb,
                                clos);
}

void
twitter_client_show_user_from_email (TwitterClient *client,
                                     const gchar   *email)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (email != NULL);

  msg = twitter_api_user_show (NULL, email);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, USER_SHOW);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, TRUE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}

void
twitter_client_show_user_from_id (TwitterClient *client,
                                  const gchar   *user)
{
  GetUserClosure *clos;
  SoupMessage *msg;

  g_return_if_fail (TWITTER_IS_CLIENT (client));
  g_return_if_fail (user != NULL);

  msg = twitter_api_user_show (user, NULL);

  clos = g_new0 (GetUserClosure, 1);
  closure_set_action (clos, USER_SHOW);
  closure_set_client (clos, g_object_ref (client));
  closure_set_requires_auth (clos, FALSE);
  clos->user = twitter_user_new ();

  twitter_client_queue_message (client, msg, TRUE,
                                get_user_cb,
                                clos);
}
