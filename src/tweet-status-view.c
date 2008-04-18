#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <clutter/cogl.h>

#include <tidy/tidy-list-view.h>
#include <tidy/tidy-list-column.h>
#include <tidy/tidy-cell-renderer.h>
#include <tidy/tidy-stylable.h>

#include "tweet-status-column.h"
#include "tweet-status-renderer.h"
#include "tweet-status-view.h"

#define TWEET_STATUS_VIEW_GET_PRIVATE(obj)      (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_STATUS_VIEW, TweetStatusViewPrivate))

struct _TweetStatusViewPrivate
{
  guint dummy : 1;
};

G_DEFINE_TYPE (TweetStatusView, tweet_status_view, TIDY_TYPE_LIST_VIEW);

static void
tweet_status_view_dispose (GObject *gobject)
{
  G_OBJECT_CLASS (tweet_status_view_parent_class)->dispose (gobject);
}

static TidyListColumn *
tweet_status_view_create_column (TidyListView *list_view,
                                 guint         model_id)
{
  return tweet_status_column_new (list_view, model_id);
}

static void
tweet_status_view_class_init (TweetStatusViewClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  TidyListViewClass *list_view_class = TIDY_LIST_VIEW_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetStatusViewPrivate));

  gobject_class->dispose = tweet_status_view_dispose;

  list_view_class->create_column = tweet_status_view_create_column;
}

static void
tweet_status_view_init (TweetStatusView *view)
{
  view->priv = TWEET_STATUS_VIEW_GET_PRIVATE (view);

  tidy_list_view_set_rules_hint (TIDY_LIST_VIEW (view), FALSE);
  tidy_list_view_set_show_headers (TIDY_LIST_VIEW (view), FALSE);

  tidy_stylable_set (TIDY_STYLABLE (view),
                     "v-padding", 6,
                     NULL);
}

ClutterActor *
tweet_status_view_new (TweetStatusModel *model)
{
  g_return_val_if_fail (TWEET_IS_STATUS_MODEL (model), NULL);

  return g_object_new (TWEET_TYPE_STATUS_VIEW,
                       "model", CLUTTER_MODEL (model),
                       NULL);
}
