/* twitter-user.c: User representation
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include <gio/gio.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libsoup/soup.h>

#include "twitter-common.h"
#include "twitter-marshal.h"
#include "twitter-private.h"
#include "twitter-user.h"

/**
 * SECTION:twitter-user
 * @show_description: A class representing a Twitter user
 *
 * #TwitterUser is a class holding the various data retrieved from Twitter
 * and describing a Twitter user.
 */

/**
 * TwitterUser:
 *
 * Representation of a Twitter user data.
 *
 * The #TwitterUser-struct struct contains private data only, and should be
 * accessed using the functions below.
 */

/**
 * TwitterUserClass:
 * @changed: class handler for the #TwitterUser::changed signal
 *
 * The #TwitterUserClass-struct contains private data only, and should be
 * accessed using the functions below.
 */

#define TWITTER_USER_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWITTER_TYPE_USER, TwitterUserPrivate))

struct _TwitterUserPrivate
{
  gchar *name;
  gchar *url;
  gchar *description;
  gchar *location;
  gchar *screen_name;
  gchar *profile_image_url;
  gchar *created_at;
  gchar *time_zone;

  guint id;
  guint friends_count;
  guint statuses_count;
  guint followers_count;
  guint favorites_count;

  gint utc_offset;

  guint protected : 1;
  guint following : 1;

  TwitterStatus *status;

  GdkPixbuf *profile_image;

  guint profile_image_load : 1;

  SoupSession *async_session;
};

enum
{
  PROP_0,

  PROP_NAME,
  PROP_URL,
  PROP_DESCRIPTION,
  PROP_LOCATION,
  PROP_SCREEN_NAME,
  PROP_PROFILE_IMAGE_URL,
  PROP_ID,
  PROP_PROTECTED,
  PROP_STATUS,
  PROP_FOLLOWING,
  PROP_FRIENDS_COUNT,
  PROP_STATUSES_COUNT,
  PROP_FOLLOWERS_COUNT,
  PROP_FAVORITES_COUNT,
  PROP_CREATED_AT,
  PROP_TIME_ZONE,
  PROP_UTC_OFFSET
};

enum
{
  CHANGED,

  LAST_SIGNAL
};

static guint user_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (TwitterUser, twitter_user, G_TYPE_INITIALLY_UNOWNED);

static void
twitter_user_finalize (GObject *gobject)
{
  TwitterUserPrivate *priv = TWITTER_USER (gobject)->priv;

  g_free (priv->name);
  g_free (priv->url);
  g_free (priv->description);
  g_free (priv->location);
  g_free (priv->screen_name);
  g_free (priv->profile_image_url);
  g_free (priv->created_at);
  g_free (priv->time_zone);

  G_OBJECT_CLASS (twitter_user_parent_class)->finalize (gobject);
}

static void
twitter_user_dispose (GObject *gobject)
{
  TwitterUserPrivate *priv = TWITTER_USER (gobject)->priv;

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  if (priv->profile_image)
    {
      g_object_unref (priv->profile_image);
      priv->profile_image = NULL;
    }

  if (priv->async_session)
    {
      soup_session_abort (priv->async_session);
      g_object_unref (priv->async_session);
      priv->async_session = NULL;
    }

  G_OBJECT_CLASS (twitter_user_parent_class)->finalize (gobject);
}

static void
twitter_user_get_property (GObject    *gobject,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  TwitterUserPrivate *priv = TWITTER_USER (gobject)->priv;

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;

    case PROP_URL:
      g_value_set_string (value, priv->url);
      break;

    case PROP_DESCRIPTION:
      g_value_set_string (value, priv->description);
      break;

    case PROP_LOCATION:
      g_value_set_string (value, priv->location);
      break;

    case PROP_SCREEN_NAME:
      g_value_set_string (value, priv->screen_name);
      break;

    case PROP_PROFILE_IMAGE_URL:
      g_value_set_string (value, priv->profile_image_url);
      break;

    case PROP_ID:
      g_value_set_uint (value, priv->id);
      break;

    case PROP_PROTECTED:
      g_value_set_boolean (value, priv->protected);
      break;

    case PROP_STATUS:
      g_value_set_object (value, priv->status);
      break;

    case PROP_FOLLOWING:
      g_value_set_boolean (value, priv->following);
      break;

    case PROP_FRIENDS_COUNT:
      g_value_set_uint (value, priv->friends_count);
      break;

    case PROP_STATUSES_COUNT:
      g_value_set_uint (value, priv->statuses_count);
      break;

    case PROP_FOLLOWERS_COUNT:
      g_value_set_uint (value, priv->followers_count);
      break;

    case PROP_CREATED_AT:
      g_value_set_string (value, priv->created_at);
      break;

    case PROP_TIME_ZONE:
      g_value_set_string (value, priv->time_zone);
      break;

    case PROP_UTC_OFFSET:
      g_value_set_int (value, priv->utc_offset);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
twitter_user_class_init (TwitterUserClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TwitterUserPrivate));

  gobject_class->get_property = twitter_user_get_property;
  gobject_class->finalize = twitter_user_finalize;
  gobject_class->dispose = twitter_user_dispose;

  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "The name of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_URL,
                                   g_param_spec_string ("url",
                                                        "URL",
                                                        "The URL of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_DESCRIPTION,
                                   g_param_spec_string ("description",
                                                        "Description",
                                                        "The description of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_LOCATION,
                                   g_param_spec_string ("location",
                                                        "Location",
                                                        "The location of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_SCREEN_NAME,
                                   g_param_spec_string ("screen-name",
                                                        "Screen Name",
                                                        "The screen name of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_PROFILE_IMAGE_URL,
                                   g_param_spec_string ("profile-image-url",
                                                        "Profile Image URL",
                                                        "The URL of the profile image of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_uint ("id",
                                                      "Id",
                                                      "The unique id of the user",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_PROTECTED,
                                   g_param_spec_boolean ("protected",
                                                         "Protected",
                                                         "Whether the user entries are protected",
                                                         FALSE,
                                                         G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_STATUS,
                                   g_param_spec_object ("status",
                                                        "Status",
                                                        "The user status",
                                                        TWITTER_TYPE_STATUS,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_FOLLOWING,
                                   g_param_spec_boolean ("following",
                                                         "Following",
                                                         "Whether we are following the user",
                                                         FALSE,
                                                         G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_FRIENDS_COUNT,
                                   g_param_spec_uint ("friends-count",
                                                      "Friends Count",
                                                      "The number of friends the user has",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_STATUSES_COUNT,
                                   g_param_spec_uint ("statuses-count",
                                                      "Statuses Count",
                                                      "The number of statuses the user wrote",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_FOLLOWERS_COUNT,
                                   g_param_spec_uint ("followers-count",
                                                      "Followers Count",
                                                      "The number of followers the user has",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_FAVORITES_COUNT,
                                   g_param_spec_uint ("favorites-count",
                                                      "Favorites Count",
                                                      "The number of favorite statues the user has",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_CREATED_AT,
                                   g_param_spec_string ("created-at",
                                                        "Created At",
                                                        "The date the user profile was created",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_TIME_ZONE,
                                   g_param_spec_string ("time-zone",
                                                        "Time Zone",
                                                        "The name of the time zone of the user",
                                                        NULL,
                                                        G_PARAM_READABLE));
  g_object_class_install_property (gobject_class,
                                   PROP_UTC_OFFSET,
                                   g_param_spec_int ("utc-offset",
                                                     "Offset from UTC",
                                                     "The offset of the time zone of the user from UTC",
                                                     G_MININT, G_MAXINT, 0,
                                                     G_PARAM_READABLE));

  user_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TwitterUserClass, changed),
                  NULL, NULL,
                  _twitter_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
twitter_user_init (TwitterUser *user)
{
  user->priv = TWITTER_USER_GET_PRIVATE (user);
}

static void
twitter_user_clean (TwitterUser *user)
{
  TwitterUserPrivate *priv = user->priv;

  g_free (priv->name);
  g_free (priv->url);
  g_free (priv->description);
  g_free (priv->location);
  g_free (priv->screen_name);
  g_free (priv->profile_image_url);
  g_free (priv->created_at);
  g_free (priv->time_zone);

  if (priv->status)
    g_object_unref (priv->status);
}

static void
twitter_user_build (TwitterUser *user,
                    JsonNode    *node)
{
  TwitterUserPrivate *priv = user->priv;
  JsonObject *obj;
  JsonNode *member;

  if (!node || JSON_NODE_TYPE (node) != JSON_NODE_OBJECT)
    return;

  obj = json_node_get_object (node);

  member = json_object_get_member (obj, "name");
  if (member)
    priv->name = json_node_dup_string (member);

  member = json_object_get_member (obj, "url");
  if (member)
    priv->url = json_node_dup_string (member);

  member = json_object_get_member (obj, "description");
  if (member)
    priv->description = json_node_dup_string (member);

  member = json_object_get_member (obj, "location");
  if (member)
    priv->location = json_node_dup_string (member);
    
  member = json_object_get_member (obj, "screen_name");
  if (member)
    priv->screen_name = json_node_dup_string (member);

  member = json_object_get_member (obj, "profile_image_url");
  if (member)
    priv->profile_image_url = json_node_dup_string (member);

  member = json_object_get_member (obj, "id");
  if (member)
    priv->id = json_node_get_int (member);

  member = json_object_get_member (obj, "protected");
  if (member)
    priv->protected = json_node_get_boolean (member);

  member = json_object_get_member (obj, "status");
  if (member)
    {
      priv->status = twitter_status_new_from_node (member);
      g_object_ref_sink (priv->status);
    }

  member = json_object_get_member (obj, "following");
  if (member)
    priv->following = json_node_get_boolean (member);

  member = json_object_get_member (obj, "friends_count");
  if (member)
    priv->friends_count = json_node_get_int (member);

  member = json_object_get_member (obj, "statuses_count");
  if (member)
    priv->statuses_count = json_node_get_int (member);

  member = json_object_get_member (obj, "followers_count");
  if (member)
    priv->followers_count = json_node_get_int (member);

  /* XXX - english spelling */
  member = json_object_get_member (obj, "favourites_count");
  if (member)
    priv->favorites_count = json_node_get_int (member);

  member = json_object_get_member (obj, "created_at");
  if (member)
    priv->created_at = json_node_dup_string (member);

  member = json_object_get_member (obj, "time_zone");
  if (member)
    priv->time_zone = json_node_dup_string (member);

  member = json_object_get_member (obj, "utc_offset");
  if (member)
    priv->utc_offset = json_node_get_int (member);
}

/**
 * twitter_user_new:
 *
 * Creates a new #TwitterUser.
 *
 * Return value: the newly created #TwitterUser. Use g_object_unref() to
 *   release the resources it allocates
 */
TwitterUser *
twitter_user_new (void)
{
  return g_object_new (TWITTER_TYPE_USER, NULL);
}

TwitterUser *
twitter_user_new_from_node (JsonNode *node)
{
  TwitterUser *retval;

  g_return_val_if_fail (node != NULL, NULL);

  retval = twitter_user_new ();
  twitter_user_build (retval, node);

  return retval;
}

/**
 * twitter_user_new_from_data:
 * @buffer: a JSON buffer holding a Twitter user
 *
 * Creates a #TwitterUser from @buffer by parsing the JSON data.
 *
 * Return value: the newly created #TwitterUser from the passed
 *   JSON data. Use g_object_unref() to release the resources
 *   it allocates
 */
TwitterUser *
twitter_user_new_from_data (const gchar *buffer)
{
  TwitterUser *retval;
  JsonParser *parser;
  GError *parse_error;

  g_return_val_if_fail (buffer != NULL, NULL);

  retval = twitter_user_new ();

  parser = json_parser_new ();
  parse_error = NULL;
  json_parser_load_from_data (parser, buffer, -1, &parse_error);
  if (parse_error)
    {
      g_warning ("Unable to parse data into a user: %s",
                 parse_error->message);
      g_error_free (parse_error);
    }
  else
    twitter_user_build (retval, json_parser_get_root (parser));

  g_object_unref (parser);

  return retval;
}

/**
 * twitter_user_load_from_data:
 * @user: a #TwitterUser
 * @buffer: a JSON buffer holding a Twitter user
 *
 * Constructs the @user properties by parsing a JSON data representation
 * of a Twitter user.
 */
void
twitter_user_load_from_data (TwitterUser *user,
                             const gchar *buffer)
{
  JsonParser *parser;
  GError *parse_error;

  g_return_if_fail (TWITTER_IS_USER (user));
  g_return_if_fail (buffer != NULL);

  twitter_user_clean (user);

  parser = json_parser_new ();
  parse_error = NULL;
  json_parser_load_from_data (parser, buffer, -1, &parse_error);
  if (parse_error)
    {
      g_warning ("Unable to parse data into a user: %s",
                 parse_error->message);
      g_error_free (parse_error);
    }
  else
    twitter_user_build (user, json_parser_get_root (parser));

  g_object_unref (parser);
}

G_CONST_RETURN gchar *
twitter_user_get_name (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->name;
}

G_CONST_RETURN gchar *
twitter_user_get_url (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->url;
}

G_CONST_RETURN gchar *
twitter_user_get_description (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->description;
}

G_CONST_RETURN gchar *
twitter_user_get_location (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->location;
}

G_CONST_RETURN gchar *
twitter_user_get_screen_name (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->screen_name;
}

G_CONST_RETURN gchar *
twitter_user_get_profile_image_url (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->profile_image_url;
}

typedef struct {
  TwitterUser *user;
  GFile *profile_image_file;
  SoupSession *async_session;
} GetProfileImageClosure;

static void
get_profile_image_vfs (GObject      *source_object,
                       GAsyncResult *res,
                       gpointer      data)
{
  GFile *file = G_FILE (source_object);
  GetProfileImageClosure *closure = data;
  TwitterUser *user = closure->user;
  GError *error;
  gchar *contents = NULL;
  gsize len;
  gchar *etags = NULL;
  GdkPixbufLoader *loader;

  error = NULL;
  g_file_load_contents_finish (file, res, &contents, &len, &etags, &error);
  if (error)
    {
      g_warning ("Unable to retrieve the contents for `%s': %s",
                 user->priv->profile_image_url,
                 error->message);
      g_error_free (error);
      goto out;
    }

  loader = gdk_pixbuf_loader_new ();
  if (!gdk_pixbuf_loader_write (loader, (const guchar *) contents, len, &error))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
        }

      g_object_unref (loader);
      g_free (contents);
      g_free (etags);

      goto out;
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
    }
  else
    {
#if 0
      /* FIXME - remove with http support for GVFS */
      gchar *user_sha1;
      gchar *cache_dir;
      gchar *cached_profile;

      user_sha1 = g_compute_checksum_for_string (G_CHECKSUM_SHA1,
                                                 user->priv->profile_image_url,
                                                 -1);
      cache_dir = g_build_filename (g_get_user_cache_dir (),
                                    "twitter-glib",
                                    "profile_images",
                                    NULL);

      if (g_mkdir_with_parents (cache_dir, 0700) == -1)
        {
          if (errno != EEXIST)
            {
              g_warning ("Unable to create the profile image cache: %s",
                         g_strerror (errno));
              g_object_unref (loader);
              g_free (contents);
              g_free (etags);
              goto out;
            }
        }

      cached_profile = g_build_filename (cache_dir, user_sha1, NULL);
      g_file_set_contents (cached_profile, contents, len, NULL);

      g_free (cached_profile);
      g_free (cache_dir);
      g_free (user_sha1);
#endif
    }

  user->priv->profile_image = gdk_pixbuf_loader_get_pixbuf (loader);
  if (user->priv->profile_image)
    g_object_ref (user->priv->profile_image);

  g_object_unref (loader);

  g_free (contents);
  g_free (etags);

  g_signal_emit (user, user_signals[CHANGED], 0);

out:
  user->priv->profile_image_load = FALSE;

  g_object_unref (closure->profile_image_file);
  g_object_unref (closure->user);
  g_free (closure);
}

static void
get_profile_image_soup (SoupSession *session,
                        SoupMessage *msg,
                        gpointer     data)
{
  GetProfileImageClosure *closure = data;
  TwitterUser *user = closure->user;
  GdkPixbufLoader *loader;
  GError *error = NULL;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      g_warning ("Unable to retrieve the contents for `%s': %s",
                 user->priv->profile_image_url,
                 msg->reason_phrase);
      goto out;
    }

  loader = gdk_pixbuf_loader_new ();

  if (!gdk_pixbuf_loader_write (loader,
                                (const guchar *) msg->response_body->data,
                                msg->response_body->length,
                                &error))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
        }

      g_object_unref (loader);
      goto out;
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      g_object_unref (loader);
    }
  else
    {
      gchar *user_sha1;
      gchar *cache_dir;
      gchar *cached_profile;

      user_sha1 = g_compute_checksum_for_string (G_CHECKSUM_SHA1,
                                                 user->priv->profile_image_url,
                                                 -1);
      cache_dir = g_build_filename (g_get_user_cache_dir (),
                                    "twitter-glib",
                                    "profile_images",
                                    NULL);

      if (g_mkdir_with_parents (cache_dir, 0700) == -1)
        {
          if (errno != EEXIST)
            {
              g_warning ("Unable to create the profile image cache: %s",
                         g_strerror (errno));
              g_object_unref (loader);
              goto out;
            }
        }

      cached_profile = g_build_filename (cache_dir, user_sha1, NULL);
      g_file_set_contents (cached_profile,
                           msg->response_body->data,
                           msg->response_body->length,
                           NULL);

      g_free (cached_profile);
      g_free (cache_dir);
      g_free (user_sha1);
    }

  user->priv->profile_image = gdk_pixbuf_loader_get_pixbuf (loader);
  if (user->priv->profile_image)
    g_object_ref (user->priv->profile_image);

  g_object_unref (loader);

  g_signal_emit (user, user_signals[CHANGED], 0);

out:
  user->priv->profile_image_load = FALSE;

  g_object_unref (closure->user);
  g_free (closure);
}

GdkPixbuf *
twitter_user_get_profile_image (TwitterUser *user)
{
  TwitterUserPrivate *priv;
  GetProfileImageClosure *closure;
  gchar *user_sha1, *cached_profile;

  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  priv = user->priv;

  if (!priv->profile_image_url)
    return NULL;

  if (priv->profile_image)
    return priv->profile_image;

  if (priv->profile_image_load)
    return NULL;

  priv->profile_image_load = TRUE;

  user_sha1 = g_compute_checksum_for_string (G_CHECKSUM_SHA1,
                                             priv->profile_image_url,
                                             -1);
  cached_profile = g_build_filename (g_get_user_cache_dir (),
                                     "twitter-glib",
                                     "profile_images",
                                     user_sha1,
                                     NULL);

  closure = g_new0 (GetProfileImageClosure, 1);
  closure->user = g_object_ref (user);

  if (g_file_test (cached_profile, G_FILE_TEST_EXISTS))
    {
      gchar *profile_image_uri;

      profile_image_uri = g_filename_to_uri (cached_profile, NULL, NULL);

      closure->profile_image_file = g_file_new_for_uri (profile_image_uri);

      g_file_load_contents_async (closure->profile_image_file,
                                  NULL,
                                  get_profile_image_vfs,
                                  closure);

      g_free (profile_image_uri);
    }
  else
    {
      SoupMessage *msg;

      if (!priv->async_session)
        priv->async_session = soup_session_async_new ();

      msg = soup_message_new (SOUP_METHOD_GET, priv->profile_image_url);

      soup_session_queue_message (priv->async_session, msg,
                                  get_profile_image_soup,
                                  closure);
    }

  g_free (user_sha1);
  g_free (cached_profile);

  return NULL;
}

guint
twitter_user_get_id (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->id;
}

gboolean
twitter_user_get_protected (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), FALSE);

  return user->priv->protected == TRUE;
}

TwitterStatus *
twitter_user_get_status (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->status;
}

gboolean
twitter_user_get_following (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), FALSE);

  return user->priv->following;
}

guint
twitter_user_get_friends_count (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->friends_count;
}

guint
twitter_user_get_statuses_count (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->statuses_count;
}

guint
twitter_user_get_followers_count (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->followers_count;
}

guint
twitter_user_get_favorites_count (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->favorites_count;
}

G_CONST_RETURN gchar *
twitter_user_get_created_at (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->created_at;
}

G_CONST_RETURN gchar *
twitter_user_get_time_zone (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), NULL);

  return user->priv->time_zone;
}

gint
twitter_user_get_utc_offset (TwitterUser *user)
{
  g_return_val_if_fail (TWITTER_IS_USER (user), 0);

  return user->priv->utc_offset;
}
