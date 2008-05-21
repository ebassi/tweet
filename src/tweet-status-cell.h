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
  ClutterActor *label;

  gchar *font_name;

  TwitterStatus *status;
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
