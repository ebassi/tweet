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

#include "twitter-common.h"
#include "twitter-enum-types.h"
#include "twitter-marshal.h"
#include "twitter-private.h"
#include "twitter-reader.h"
#include "twitter-status.h"
#include "twitter-timeline.h"
#include "twitter-user.h"

#define TWITTER_API_PUBLIC_TIMELINE     "http://twitter.com/statuses/public_timeline.json"
#define TWITTER_API_FRIENDS_TIMELINE    "http://twitter.com/statuses/friends_timeline"
#define TWITTER_API_SHOW                "http://twitter.com/statuses/show/"

#define TWITTER_READER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWITTER_TYPE_READER, TwitterReaderPrivate))

struct _TwitterReaderPrivate
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
  TIMELINE_RECEIVED,
  STATUS_RECEIVED,

  LAST_SIGNAL
};

static guint reader_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (TwitterReader, twitter_reader, G_TYPE_OBJECT);

static void
twitter_reader_finalize (GObject *gobject)
{
  TwitterReaderPrivate *priv = TWITTER_READER (gobject)->priv;

  soup_session_abort (priv->session_async);
  g_object_unref (priv->session_async);

  soup_session_abort (priv->session_sync);
  g_object_unref (priv->session_sync);

  g_free (priv->email);
  g_free (priv->password);

  G_OBJECT_CLASS (twitter_reader_parent_class)->finalize (gobject);
}

static void
twitter_reader_set_property (GObject      *gobject,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{

}

static void
twitter_reader_get_property (GObject    *gobject,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{

}

static void
twitter_reader_class_init (TwitterReaderClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TwitterReaderPrivate));

  gobject_class->set_property = twitter_reader_set_property;
  gobject_class->get_property = twitter_reader_get_property;
  gobject_class->finalize = twitter_reader_finalize;

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

  reader_signals[AUTHENTICATE] =
    g_signal_new ("authenticate",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                  G_STRUCT_OFFSET (TwitterReaderClass, authenticate),
                  NULL, NULL,
                  _twitter_marshal_BOOLEAN__ENUM,
                  G_TYPE_BOOLEAN, 1,
                  TWITTER_TYPE_AUTH_STATE);
  reader_signals[TIMELINE_RECEIVED] =
    g_signal_new ("timeline-received",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterReaderClass, timeline_received),
                  NULL, NULL,
                  _twitter_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  TWITTER_TYPE_TIMELINE,
                  G_TYPE_POINTER);
  reader_signals[STATUS_RECEIVED] =
    g_signal_new ("status-received",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterReaderClass, status_received),
                  NULL, NULL,
                  _twitter_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  TWITTER_TYPE_STATUS,
                  G_TYPE_POINTER);
}

static void
twitter_reader_init (TwitterReader *reader)
{
  TwitterReaderPrivate *priv;

  reader->priv = priv = TWITTER_READER_GET_PRIVATE (reader);

  priv->session_sync =
    soup_session_sync_new_with_options ("user-agent", "Twitter-GLib/" VERSION,
                                        NULL);
  priv->session_async =
    soup_session_async_new_with_options ("user-agent", "Twitter-GLib/" VERSION,
                                         NULL);
}

TwitterReader *
twitter_reader_new (void)
{
  return g_object_new (TWITTER_TYPE_READER, NULL);
}

TwitterReader *
twitter_reader_new_for_user (const gchar *email,
                             const gchar *password)
{
  return g_object_new (TWITTER_TYPE_READER,
                       "email", email,
                       "password", password,
                       NULL);
}

typedef enum {
  PUBLIC_TIMELINE,
  FRIENDS_TIMELINE,
  USER_TIMELINE,
  REPLIES,
  SHOW
} ReaderAction;

typedef struct {
  ReaderAction action;
  TwitterReader *reader;
  TwitterTimeline *timeline;
} GetTimelineClosure;

typedef struct {
  ReaderAction action;
  TwitterReader *reader;
  TwitterStatus *status;
} GetStatusClosure;

static void
get_timeline_cb (SoupSession *session,
                 SoupMessage *msg,
                 gpointer     user_data)
{
  GetTimelineClosure *closure = user_data;
  TwitterReader *reader = closure->reader; 
  guint status;

  status = msg->status_code;
  if (!SOUP_STATUS_IS_SUCCESSFUL (status))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (status),
                   msg->reason_phrase);

      g_signal_emit (reader, reader_signals[TIMELINE_RECEIVED], 0,
                     closure->timeline, error);
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

      g_signal_emit (reader, reader_signals[TIMELINE_RECEIVED], 0,
                     closure->timeline, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->timeline);
  g_object_unref (closure->reader);

  g_free (closure);
}

static void
get_status_cb (SoupSession *session,
               SoupMessage *msg,
               gpointer     user_data)
{
  GetStatusClosure *closure = user_data;
  TwitterReader *reader = closure->reader; 

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      GError *error = NULL;

      g_set_error (&error, TWITTER_ERROR,
                   twitter_error_from_status (msg->status_code),
                   msg->reason_phrase);

      g_signal_emit (reader, reader_signals[STATUS_RECEIVED], 0,
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

      g_signal_emit (reader, reader_signals[STATUS_RECEIVED], 0,
                     closure->status, NULL);

      g_free (buffer);
    }

  g_object_unref (closure->reader);
  g_object_unref (closure->status);

  g_free (closure);
}

static void
twitter_reader_auth (SoupSession *session,
                     SoupMessage *msg,
                     SoupAuth    *auth,
                     gboolean     retrying,
                     gpointer     user_data)
{
  TwitterReader *reader = user_data;
  TwitterReaderPrivate *priv = reader->priv;
  gboolean retval = FALSE;

  if (!retrying)
    {
      g_signal_emit (reader, reader_signals[AUTHENTICATE], 0,
                     TWITTER_AUTH_NEGOTIATING, &retval);

      soup_auth_authenticate (auth, priv->email, priv->password);

      priv->auth_complete = TRUE;
    }
  else
    {
      g_signal_emit (reader, reader_signals[AUTHENTICATE], 0,
                     TWITTER_AUTH_RETRY, &retval);

      if (G_LIKELY (retval))
        {
          soup_auth_authenticate (auth, priv->email, priv->password);
          priv->auth_complete = TRUE;
        }
      else
        {
          g_signal_emit (reader, reader_signals[AUTHENTICATE], 0,
                         TWITTER_AUTH_FAILED, &retval);
          priv->auth_complete = FALSE;
        }
    }
}

static void
twitter_reader_queue_message (TwitterReader       *reader,
                              const gchar         *url,
                              gboolean             requires_auth,
                              SoupSessionCallback  callback,
                              gpointer             data)
{
  TwitterReaderPrivate *priv = reader->priv;
  SoupMessage *msg;

  msg = soup_message_new (SOUP_METHOD_GET, url);

  if (requires_auth && !priv->auth_id)
    priv->auth_id = g_signal_connect (priv->session_async, "authenticate",
                                      G_CALLBACK (twitter_reader_auth),
                                      reader);

  soup_session_queue_message (priv->session_async, msg,
                              callback,
                              data);
}

void
twitter_reader_get_public_timeline (TwitterReader *reader,
                                    guint          since_id)
{
  GetTimelineClosure *clos;
  gchar *url;

  g_return_if_fail (TWITTER_IS_READER (reader));

  if (since_id > 0)
    url = g_strdup_printf ("%s?since=%u",
                           TWITTER_API_PUBLIC_TIMELINE,
                           since_id);
  else
    url = g_strdup (TWITTER_API_PUBLIC_TIMELINE);

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = PUBLIC_TIMELINE;
  clos->reader = g_object_ref (reader);
  clos->timeline = twitter_timeline_new ();

  twitter_reader_queue_message (reader, url, FALSE,
                                get_timeline_cb,
                                clos);

  g_free (url);
}

void
twitter_reader_get_friends_timeline (TwitterReader *reader,
                                     const gchar   *friend_,
                                     const gchar   *since_date)
{
  GetTimelineClosure *clos;
  gchar *base_url, *url;

  g_return_if_fail (TWITTER_IS_READER (reader));

  if (friend_ && *friend_ != '\0')
    base_url = g_strconcat (TWITTER_API_FRIENDS_TIMELINE "/",
                            friend_, ".json",
                            NULL);
  else
    base_url = g_strdup (TWITTER_API_FRIENDS_TIMELINE ".json");

  if (since_date && *since_date != '\0')
    url = g_strconcat (base_url, "?since=\"", since_date, "\"", NULL);
  else
    url = base_url;

  clos = g_new0 (GetTimelineClosure, 1);
  clos->action = FRIENDS_TIMELINE;
  clos->reader = g_object_ref (reader);
  clos->timeline = twitter_timeline_new ();

  twitter_reader_queue_message (reader, url, FALSE,
                                get_timeline_cb,
                                clos);

  if (base_url != url)
    g_free (url);

  g_free (base_url);
}

void
twitter_reader_show (TwitterReader *reader,
                     guint          status_id)
{
  GetStatusClosure *clos;
  gchar *url;

  g_return_if_fail (TWITTER_IS_READER (reader));
  g_return_if_fail (status_id > 0);

  url = g_strdup_printf ("%s/%u.json", TWITTER_API_SHOW, status_id);

  clos = g_new0 (GetStatusClosure, 1);
  clos->action = SHOW;
  clos->reader = g_object_ref (reader);
  clos->status = twitter_status_new ();

  twitter_reader_queue_message (reader, url, FALSE,
                                get_status_cb,
                                clos);

  g_free (url);
}
