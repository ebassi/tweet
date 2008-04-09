#ifndef __TWITTER_PRIVATE_H__
#define __TWITTER_PRIVATE_H__

#include <json-glib/json-glib.h>
#include "twitter-status.h"
#include "twitter-user.h"

G_BEGIN_DECLS

TwitterStatus *twitter_status_new_from_node (JsonNode *node);
TwitterUser   *twitter_user_new_from_node   (JsonNode *node);

G_END_DECLS

#endif /* __TWITTER_PRIVATE_H__ */
