/* twitter-private.h: Private definitions
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
