#ifndef __TWEET_AUTH_DIALOG_H__
#define __TWEET_AUTH_DIALOG_H__

#include <gtk/gtkdialog.h>

G_BEGIN_DECLS

#define TWEET_TYPE_AUTH_DIALOG                  (tweet_auth_dialog_get_type ())
#define TWEET_AUTH_DIALOG(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_AUTH_DIALOG, TweetAuthDialog))
#define TWEET_IS_AUTH_DIALOG(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_AUTH_DIALOG))
#define TWEET_AUTH_DIALOG_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_AUTH_DIALOG, TweetAuthDialogClass))
#define TWEET_IS_AUTH_DIALOG_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_AUTH_DIALOG))
#define TWEET_AUTH_DIALOG_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_AUTH_DIALOG, TweetAuthDialogClass))

typedef struct _TweetAuthDialog         TweetAuthDialog;
typedef struct _TweetAuthDialogPrivate  TweetAuthDialogPrivate;
typedef struct _TweetAuthDialogClass    TweetAuthDialogClass;

struct _TweetAuthDialog
{
  GtkDialog parent_instance;

  TweetAuthDialogPrivate *priv;
};

struct _TweetAuthDialogClass
{
  GtkDialogClass parent_class;
};

GType                 tweet_auth_dialog_get_type     (void) G_GNUC_CONST;
GtkWidget *           tweet_auth_dialog_new          (GtkWidget       *parent,
                                                      const gchar     *title);
G_CONST_RETURN gchar *tweet_auth_dialog_get_username (TweetAuthDialog *dialog);
G_CONST_RETURN gchar *tweet_auth_dialog_get_password (TweetAuthDialog *dialog);

G_END_DECLS

#endif /* __TWEET_AUTH_DIALOG_H__ */
