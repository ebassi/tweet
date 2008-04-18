#ifndef __TWEET_WINDOW_H__
#define __TWEET_WINDOW_H__

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

#define TWEET_TYPE_WINDOW               (tweet_window_get_type ())
#define TWEET_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_WINDOW, TweetWindow))
#define TWEET_IS_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_WINDOW))
#define TWEET_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_WINDOW, TweetWindowClass))
#define TWEET_IS_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_WINDOW))
#define TWEET_WINDOW_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_WINDOW, TweetWindowClass))

typedef struct _TweetWindow             TweetWindow;
typedef struct _TweetWindowPrivate      TweetWindowPrivate;
typedef struct _TweetWindowClass        TweetWindowClass;

struct _TweetWindow
{
  GtkWindow parent_instance;

  TweetWindowPrivate *priv;
};

struct _TweetWindowClass
{
  GtkWindowClass parent_class;
};

GType      tweet_window_get_type (void) G_GNUC_CONST;
GtkWidget *tweet_window_new      (void);

G_END_DECLS

#endif /* __TWEET_WINDOW_H__ */
