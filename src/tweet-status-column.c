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
