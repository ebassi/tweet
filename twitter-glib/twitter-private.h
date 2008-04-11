#ifndef __TWITTER_PRIVATE_H__
#define __TWITTER_PRIVATE_H__

#include <json-glib/json-glib.h>
#include "twitter-status.h"
#include "twitter-user.h"

G_BEGIN_DECLS

#define I_(str) (g_intern_static_string ((str)))

TwitterStatus *twitter_status_new_from_node (JsonNode *node);
TwitterUser   *twitter_user_new_from_node   (JsonNode *node);

G_END_DECLS

#endif /* __TWITTER_PRIVATE_H__ */
