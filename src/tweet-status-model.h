#ifndef __TWEET_STATUS_MODEL_H__
#define __TWEET_STATUS_MODEL_H__

#include <clutter/clutter-model.h>
#include <twitter-glib/twitter-glib.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_MODEL                 (tweet_status_model_get_type ())
#define TWEET_STATUS_MODEL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_MODEL, TweetStatusModel))
#define TWEET_IS_STATUS_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_MODEL))
#define TWEET_STATUS_MODEL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_MODEL, TweetStatusModelClass))
#define TWEET_IS_STATUS_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_MODEL))
#define TWEET_STATUS_MODEL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_MODEL, TweetStatusModelClass))

typedef struct _TweetStatusModel              TweetStatusModel;
typedef struct _TweetStatusModelPrivate       TweetStatusModelPrivate;
typedef struct _TweetStatusModelClass         TweetStatusModelClass;

struct _TweetStatusModel
{
  ClutterModel parent_instance;

  TweetStatusModelPrivate *priv;
};

struct _TweetStatusModelClass
{
  ClutterModelClass parent_class;
};

GType tweet_status_model_get_type (void);

ClutterModel *tweet_status_model_new            (void);
void          tweet_status_model_append_status  (TweetStatusModel *model,
                                                 TwitterStatus    *status);
void          tweet_status_model_prepend_status (TweetStatusModel *model,
                                                 TwitterStatus    *status);
void          tweet_status_model_set_max_size   (TweetStatusModel *model,
                                                 gint              max_size);

G_END_DECLS

#endif /* __TWEET_STATUS_MODEL_H__ */
