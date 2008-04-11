#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glib.h>

#include <json-glib/json-glib.h>

#include "twitter-common.h"
#include "twitter-enum-types.h"
#include "twitter-private.h"
#include "twitter-status.h"
#include "twitter-user-list.h"
#include "twitter-user.h"

#define TWITTER_USER_LIST_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWITTER_TYPE_USER_LIST, TwitterUserListPrivate))

struct _TwitterUserListPrivate
{
  GHashTable *user_by_id;
  GList *user_list;
};

G_DEFINE_TYPE (TwitterUserList, twitter_user_list, G_TYPE_OBJECT);

static void
twitter_user_list_finalize (GObject *gobject)
{
  TwitterUserListPrivate *priv = TWITTER_USER_LIST (gobject)->priv;

  g_hash_table_destroy (priv->user_by_id);
  g_list_free (priv->user_list);

  G_OBJECT_CLASS (twitter_user_list_parent_class)->finalize (gobject);
}

static void
twitter_user_list_class_init (TwitterUserListClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TwitterUserListPrivate));

  gobject_class->finalize = twitter_user_list_finalize;
}

static void
twitter_user_list_init (TwitterUserList *user_list)
{
  TwitterUserListPrivate *priv;

  user_list->priv = priv = TWITTER_USER_LIST_GET_PRIVATE (user_list);

  priv->user_by_id = g_hash_table_new_full (NULL, NULL,
                                            NULL,
                                            g_object_unref);
}

static void
twitter_user_list_clean (TwitterUserList *user_list)
{
  TwitterUserListPrivate *priv = user_list->priv;

  if (priv->user_list)
    {
      g_list_free (priv->user_list);
      priv->user_list = NULL;
    }

  if (priv->user_by_id)
    {
      g_hash_table_destroy (priv->user_by_id);
      priv->user_by_id = g_hash_table_new_full (NULL, NULL,
                                                NULL,
                                                g_object_unref);
    }
}

static void
twitter_user_list_build (TwitterUserList *user_list,
                        JsonNode        *node)
{
  TwitterUserListPrivate *priv = user_list->priv;
  JsonArray *array;
  GList *elements, *l;
  GList *list = NULL;

  if (!node || JSON_NODE_TYPE (node) != JSON_NODE_ARRAY)
    return;

  array = json_node_get_array (node);
  elements = json_array_get_elements (array);

  for (l = elements; l != NULL; l = l->next)
    {
      JsonNode *element = l->data;

      if (JSON_NODE_TYPE (element) == JSON_NODE_OBJECT)
        {
          TwitterUser *user;
          guint user_id;

          user = twitter_user_new_from_node (element);
          user_id = twitter_user_get_id (user);
          if (user_id == 0)
            {
              g_object_unref (user);
              continue;
            }

          g_hash_table_replace (priv->user_by_id,
                                GUINT_TO_POINTER (user_id),
                                g_object_ref_sink (user));
          list = g_list_prepend (list, user);
        }
    }

  priv->user_list = g_list_reverse (list);
}

TwitterUserList *
twitter_user_list_new (void)
{
  return g_object_new (TWITTER_TYPE_USER_LIST, NULL);
}

TwitterUserList *
twitter_user_list_new_from_data (const gchar *buffer)
{
  TwitterUserList *retval;
  JsonParser *parser;
  GError *parse_error;

  g_return_val_if_fail (buffer != NULL, NULL);

  retval = twitter_user_list_new ();

  parser = json_parser_new ();
  parse_error = NULL;
  json_parser_load_from_data (parser, buffer, -1, &parse_error);
  if (parse_error)
    {
      g_warning ("Unable to parse data into a user list: %s",
                 parse_error->message);
      g_error_free (parse_error);
    }
  else
    twitter_user_list_build (retval, json_parser_get_root (parser));

  g_object_unref (parser);

  return retval;
}

void
twitter_user_list_load_from_data (TwitterUserList *user_list,
                                 const gchar     *buffer)
{
  JsonParser *parser;
  GError *parse_error;

  g_return_if_fail (TWITTER_IS_USER_LIST (user_list));
  g_return_if_fail (buffer != NULL);

  twitter_user_list_clean (user_list);

  parser = json_parser_new ();
  parse_error = NULL;
  json_parser_load_from_data (parser, buffer, -1, &parse_error);
  if (parse_error)
    {
      g_warning ("Unable to parse data into a user list: %s",
                 parse_error->message);
      g_error_free (parse_error);
    }
  else
    twitter_user_list_build (user_list, json_parser_get_root (parser));

  g_object_unref (parser);
}

guint
twitter_user_list_get_count (TwitterUserList *user_list)
{
  g_return_val_if_fail (TWITTER_IS_USER_LIST (user_list), 0);

  return g_hash_table_size (user_list->priv->user_by_id);
}

TwitterUser *
twitter_user_list_get_id (TwitterUserList *user_list,
                          guint            id)
{
  g_return_val_if_fail (TWITTER_IS_USER_LIST (user_list), NULL);

  return g_hash_table_lookup (user_list->priv->user_by_id,
                              GUINT_TO_POINTER (id));
}

TwitterUser *
twitter_user_list_get_pos (TwitterUserList *user_list,
                           gint             index_)
{
  g_return_val_if_fail (TWITTER_IS_USER_LIST (user_list), NULL);
  g_return_val_if_fail (ABS (index_) < twitter_user_list_get_count (user_list), NULL);

  if (index_ >= 0)
    return g_list_nth_data (user_list->priv->user_list, index_);
  else
    {
      guint roll = g_list_length (user_list->priv->user_list);

      roll += index_;
      return g_list_nth_data (user_list->priv->user_list, roll);
    }
}

GList *
twitter_user_list_get_all (TwitterUserList *user_list)
{
  g_return_val_if_fail (TWITTER_IS_USER_LIST (user_list), NULL);

  return g_list_copy (user_list->priv->user_list);
}
