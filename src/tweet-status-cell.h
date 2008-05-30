/* tweet-status-cell.h: Cell actor for the status renderer
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
#ifndef __TWEET_STATUS_CELL_H__
#define __TWEET_STATUS_CELL_H__

#include <clutter/clutter-group.h>
#include <twitter-glib/twitter-glib.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_CELL                  (tweet_status_cell_get_type ())
#define TWEET_STATUS_CELL(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_CELL, TweetStatusCell))
#define TWEET_IS_STATUS_CELL(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_CELL))
#define TWEET_STATUS_CELL_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_CELL, TweetStatusCellClass))
#define TWEET_IS_STATUS_CELL_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_CELL))
#define TWEET_STATUS_CELL_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_CELL, TweetStatusCellClass))

typedef struct _TweetStatusCell             TweetStatusCell;
typedef struct _TweetStatusCellClass        TweetStatusCellClass;

struct _TweetStatusCell
{
  ClutterGroup parent_instance;

  ClutterActor *bg;
  ClutterActor *icon;
  ClutterActor *bubble;
  ClutterActor *label;

  gchar *font_name;

  GRegex *escape_re;

  TwitterStatus *status;

  ClutterUnit cell_height;
};

struct _TweetStatusCellClass
{
  ClutterGroupClass parent_class;
};

GType         tweet_status_cell_get_type (void) G_GNUC_CONST;
ClutterActor *tweet_status_cell_new      (TwitterStatus *status,
                                          const gchar   *font_name);

G_END_DECLS

#endif /* __TWEET_STATUS_CELL_H__ */
