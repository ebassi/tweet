/* tweet-hot-actor.c: Interface to an actor with a dynamic cursor
 *
 * This file is part of Tweet.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>
#include <gdk/gdkdisplay.h>
#include <gdk/gdkcursor.h>

#include "tweet-hot-actor.h"

GType
tweet_hot_actor_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (!type))
    {
      GTypeInfo container_info =
	{
	  sizeof (TweetHotActorIface),
	  NULL, /* iface_base_init */
	  NULL, /* iface_base_finalize */
	};

      type = g_type_register_static (G_TYPE_INTERFACE,
				     g_intern_static_string ("TweetHotActor"),
				     &container_info, 0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }

  return type;
}

GdkCursor *
tweet_hot_actor_get_cursor (TweetHotActor *actor,
			    GdkDisplay    *display,
			    int            x,
			    int            y)
{
  g_return_val_if_fail (TWEET_IS_HOT_ACTOR (actor), NULL);

  return TWEET_HOT_ACTOR_GET_IFACE (actor)->get_cursor (actor,
							display,
							x, y);
}
