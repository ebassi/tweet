#ifndef __TWEET_STATUS_VIEW_H__
#define __TWEET_STATUS_VIEW_H__

#include <tidy/tidy-list-view.h>
#include "tweet-status-model.h"

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_VIEW                  (tweet_status_view_get_type ())
#define TWEET_STATUS_VIEW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_VIEW, TweetStatusView))
#define TWEET_IS_STATUS_VIEW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_VIEW))
#define TWEET_STATUS_VIEW_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_VIEW, TweetStatusViewClass))
#define TWEET_IS_STATUS_VIEW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_VIEW))
#define TWEET_STATUS_VIEW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_VIEW, TweetStatusViewClass))

typedef struct _TweetStatusView         TweetStatusView;
typedef struct _TweetStatusViewPrivate  TweetStatusViewPrivate;
typedef struct _TweetStatusViewClass    TweetStatusViewClass;

struct _TweetStatusView
{
  TidyListView parent_instance;

  TweetStatusViewPrivate *priv;
};

struct _TweetStatusViewClass
{
  TidyListViewClass parent_class;
};

GType         tweet_status_view_get_type (void) G_GNUC_CONST;
ClutterActor *tweet_status_view_new      (TweetStatusModel *model);

G_END_DECLS

#endif /* __TWEET_STATUS_VIEW_H__ */
