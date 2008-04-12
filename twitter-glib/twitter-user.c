/* twitter-user.c - Twitter user representation
 * Copyright (C) 2008 - Emmanuele Bassi <ebassi@gnome.org>
 *
 * This file is part of twitter-glib.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include "twitter-common.h"
#include "twitter-private.h"
#include "twitter-user.h"

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
