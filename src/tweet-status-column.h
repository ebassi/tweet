/* tweet-status-column.h: Column for the status view
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
#ifndef __TWEET_STATUS_COLUMN_H__
#define __TWEET_STATUS_COLUMN_H__

#include <tidy/tidy-list-column.h>
#include <tidy/tidy-list-view.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_COLUMN                (tweet_status_column_get_type ())
#define TWEET_STATUS_COLUMN(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_COLUMN, TweetStatusColumn))
#define TWEET_IS_STATUS_COLUMN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_COLUMN))
#define TWEET_STATUS_COLUMN_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_COLUMN, TweetStatusColumnClass))
#define TWEET_IS_STATUS_COLUMN_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_COLUMN))
#define TWEET_STATUS_COLUMN_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_COLUMN, TweetStatusColumnClass))

typedef struct _TweetStatusColumn               TweetStatusColumn;
typedef struct _TweetStatusColumnClass          TweetStatusColumnClass;

struct _TweetStatusColumn
{
  TidyListColumn parent_instance;
};

struct _TweetStatusColumnClass
{
  TidyListColumnClass parent_class;
};

GType           tweet_status_column_get_type (void) G_GNUC_CONST;
TidyListColumn *tweet_status_column_new      (TidyListView *list_view,
                                              guint         model_index);

G_END_DECLS

#endif /* __TWEET_STATUS_COLUMN_H__ */
