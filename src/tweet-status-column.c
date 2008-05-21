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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include <tidy/tidy-cell-renderer.h>
#include <tidy/tidy-list-column.h>

#include "tweet-status-column.h"
#include "tweet-status-renderer.h"

G_DEFINE_TYPE (TweetStatusColumn, tweet_status_column, TIDY_TYPE_LIST_COLUMN);

static void
tweet_status_column_class_init (TweetStatusColumnClass *klass)
{

}

static void
tweet_status_column_init (TweetStatusColumn *status_column)
{
  TidyListColumn *column = TIDY_LIST_COLUMN (status_column);

  /* at least the avatar and some space for a text ellipsis */
  tidy_list_column_set_min_width (column, 192);
  tidy_list_column_set_max_width (column, 350);

  /* we provide our own renderers */
  tidy_list_column_set_cell_renderer (column, tweet_status_renderer_new ());
  tidy_list_column_set_header_renderer (column, tweet_status_renderer_new ());
}

TidyListColumn *
tweet_status_column_new (TidyListView *list_view,
                         guint         model_index)
{
  g_return_val_if_fail (TIDY_IS_LIST_VIEW (list_view), NULL);

  return g_object_new (TWEET_TYPE_STATUS_COLUMN,
                       "list-view", list_view,
                       "model-index", model_index,
                       NULL);
}
