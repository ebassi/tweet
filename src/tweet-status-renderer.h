/* tweet-status-renderer.h: Renderer for the status view
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
#ifndef __TWEET_STATUS_RENDERER_H__
#define __TWEET_STATUS_RENDERER_H__

#include <glib-object.h>
#include <tidy/tidy-cell-renderer.h>
#include <twitter-glib/twitter-glib.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_RENDERER              (tweet_status_renderer_get_type ())
#define TWEET_STATUS_RENDERER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_RENDERER, TweetStatusRenderer))
#define TWEET_IS_STATUS_RENDERER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_RENDERER))
#define TWEET_STATUS_RENDERER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_RENDERER, TweetStatusRendererClass))
#define TWEET_IS_STATUS_RENDERER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_RENDERER))
#define TWEET_STATUS_RENDERER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_RENDERER, TweetStatusRendererClass))

typedef struct _TweetStatusRenderer             TweetStatusRenderer;
typedef struct _TweetStatusRendererClass        TweetStatusRendererClass;

struct _TweetStatusRenderer
{
  TidyCellRenderer parent_instance;
};

struct _TweetStatusRendererClass
{
  TidyCellRendererClass parent_class;
};

GType             tweet_status_renderer_get_type (void) G_GNUC_CONST;
TidyCellRenderer *tweet_status_renderer_new      (void);

G_END_DECLS

#endif /* __TWEET_STATUS_RENDERER_H__ */
