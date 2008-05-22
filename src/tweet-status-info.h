#ifndef __TWEET_STATUS_INFO_H__
#define __TWEET_STATUS_INFO_H__

#include <tidy/tidy-actor.h>

G_BEGIN_DECLS

#define TWEET_TYPE_STATUS_INFO                  (tweet_status_info_get_type ())
#define TWEET_STATUS_INFO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_STATUS_INFO, TweetStatusInfo))
#define TWEET_IS_STATUS_INFO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_STATUS_INFO))
#define TWEET_STATUS_INFO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_STATUS_INFO, TweetStatusInfoClass))
#define TWEET_IS_STATUS_INFO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_STATUS_INFO))
#define TWEET_STATUS_INFO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_STATUS_INFO, TweetStatusInfoClass))

typedef struct _TweetStatusInfo         TweetStatusInfo;
typedef struct _TweetStatusInfoClass    TweetStatusInfoClass;

struct _TweetStatusInfo
{
  TidyActor parent_instance;

  ClutterActorBox allocation;

  ClutterActor *bg;
  ClutterActor *icon;
  ClutterActor *label;
  ClutterActor *reply_button;
  ClutterActor *star_button;

  GRegex *escape_re;

  TwitterStatus *status;
};

struct _TweetStatusInfoClass
{
  TidyActorClass parent_class;
};

GType          tweet_status_info_get_type   (void) G_GNUC_CONST;
ClutterActor * tweet_status_info_new        (TwitterStatus   *status);
void           tweet_status_info_set_status (TweetStatusInfo *info,
                                             TwitterStatus   *status);
TwitterStatus *tweet_status_info_get_status (TweetStatusInfo *info);

G_END_DECLS

#endif /* __TWEET_STATUS_INFO_H__ */
