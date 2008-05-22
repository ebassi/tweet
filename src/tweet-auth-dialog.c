#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <gtk/gtk.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-auth-dialog.h"
#include "tweet-config.h"
#include "tweet-utils.h"

#define TWEET_AUTH_DIALOG_GET_PRIVATE(obj)      (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_AUTH_DIALOG, TweetAuthDialogPrivate))

struct _TweetAuthDialogPrivate
{
  GtkWidget *email_entry;
  GtkWidget *password_entry;

  GtkWidget *ok_button;
  GtkWidget *verify_button;
  GtkWidget *close_button;

  GtkWidget *verify_label;

  GtkSizeGroup *size_group;

  TweetConfig *config;

  TwitterClient *client;

  guint has_email    : 1;
  guint has_password : 1;
};

G_DEFINE_TYPE (TweetAuthDialog, tweet_auth_dialog, GTK_TYPE_DIALOG);

static void
tweet_auth_dialog_dispose (GObject *gobject)
{
  TweetAuthDialogPrivate *priv = TWEET_AUTH_DIALOG (gobject)->priv;

  if (priv->client)
    {
      g_object_unref (priv->client);
      priv->client = NULL;
    }

  if (priv->size_group)
    {
      g_object_unref (priv->size_group);
      priv->size_group = NULL;
    }

  G_OBJECT_CLASS (tweet_auth_dialog_parent_class)->dispose (gobject);
}

static void
on_user_verified (TwitterClient   *client,
                  gboolean         is_verified,
                  const GError    *error,
                  TweetAuthDialog *dialog)
{
  gchar *text;

  if (error)
    {
      text = g_strdup_printf ("<i>Unable to verify user: %s</i>", error->message);
      gtk_label_set_text (GTK_LABEL (dialog->priv->verify_label), text);
      gtk_label_set_use_markup (GTK_LABEL (dialog->priv->verify_label), TRUE);
      g_free (text);
    }
  else
    {
      if (is_verified)
        text = "<i>User verified</i>";
      else
        text = "<i>Invalid user</i>";

      gtk_label_set_text (GTK_LABEL (dialog->priv->verify_label), text);
      gtk_label_set_use_markup (GTK_LABEL (dialog->priv->verify_label), TRUE);
    }

  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

static void
on_response (GtkDialog *dialog,
             gint       response_id)
{
  TweetAuthDialogPrivate *priv = TWEET_AUTH_DIALOG (dialog)->priv;

  if (response_id == 1)
    {
      const gchar *email, *password;

      g_signal_stop_emission_by_name (dialog, "response");

      gtk_dialog_set_response_sensitive (dialog, 1, FALSE);
      gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_OK, FALSE);

      gtk_label_set_text (GTK_LABEL (priv->verify_label),
                          "<i>Verifying credentials...</i>");
      gtk_label_set_use_markup (GTK_LABEL (priv->verify_label), TRUE);
      gtk_widget_show (priv->verify_label);

      email = gtk_entry_get_text (GTK_ENTRY (priv->email_entry));
      password = gtk_entry_get_text (GTK_ENTRY (priv->password_entry));

      if (!priv->client)
        {
          priv->client = twitter_client_new_for_user (email, password);
          g_signal_connect (priv->client,
                            "user-verified", G_CALLBACK (on_user_verified),
                            dialog);
        }
      else
        twitter_client_set_user (priv->client, email, password);

      twitter_client_verify_user (priv->client);
    }
}

static void
on_email_changed (GtkEntry *entry,
                  TweetAuthDialog *dialog)
{
  TweetAuthDialogPrivate *priv = dialog->priv;
  const gchar *email = gtk_entry_get_text (entry);

  if (strlen (email) == 0)
    {
      priv->has_email = FALSE;
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), 1, FALSE);
    }
  else
    {
      priv->has_email = TRUE;

      if (priv->has_password)
        {
          gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
          gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), 1, TRUE);
        }
    }
}

static void
on_password_changed (GtkEntry *entry,
                     TweetAuthDialog *dialog)
{
  TweetAuthDialogPrivate *priv = dialog->priv;
  const gchar *password = gtk_entry_get_text (entry);

  if (strlen (password) == 0)
    {
      priv->has_password = FALSE;
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), 1, FALSE);
    }
  else
    {
      priv->has_password = TRUE;

      if (priv->has_email)
        {
          gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
          gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), 1, TRUE);
        }
    }

}

static void
tweet_auth_dialog_constructed (GObject *gobject)
{
  TweetAuthDialog *dialog = TWEET_AUTH_DIALOG (gobject);
  TweetAuthDialogPrivate *priv = dialog->priv;
  GtkWidget *main_hbox;
  GtkWidget *hbox, *vbox;
  GtkWidget *label;
  GtkWidget *image;

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 2);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  g_signal_connect (dialog, "response", G_CALLBACK (on_response), NULL);

  main_hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                      main_hbox,
                      FALSE, FALSE, 0);
  gtk_widget_show (main_hbox);

  image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_AUTHENTICATION,
                                    GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (main_hbox), image, TRUE, TRUE, 0);
  gtk_widget_show (image);

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_box_pack_end (GTK_BOX (main_hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  priv->size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  label = gtk_label_new ("Please insert the email address and password\n"
                         "used when registering the account on Twitter.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  /* email entry */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new ("Email address:");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget (priv->size_group, label);
  gtk_widget_show (label);

  priv->email_entry = gtk_entry_new ();
  g_signal_connect (priv->email_entry,
                    "changed", G_CALLBACK (on_email_changed),
                    dialog);
  gtk_box_pack_end (GTK_BOX (hbox), priv->email_entry, TRUE, TRUE, 0);
  gtk_widget_show (priv->email_entry);

  /* password entry */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new ("Password:");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_size_group_add_widget (priv->size_group, label);
  gtk_widget_show (label);

  priv->password_entry = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (priv->password_entry), FALSE);
  gtk_entry_set_invisible_char (GTK_ENTRY (priv->password_entry), '*');
  g_signal_connect (priv->password_entry,
                    "changed", G_CALLBACK (on_password_changed),
                    dialog);
  gtk_box_pack_end (GTK_BOX (hbox), priv->password_entry, TRUE, TRUE, 0);
  gtk_widget_show (priv->password_entry);

  priv->verify_label = gtk_label_new ("");
  gtk_box_pack_end (GTK_BOX (vbox), priv->verify_label, FALSE, FALSE, 0);
  gtk_widget_show (priv->verify_label);

  /* buttons */
  priv->ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
                                           GTK_STOCK_OK,
                                           GTK_RESPONSE_OK);
  gtk_widget_set_sensitive (priv->ok_button, FALSE);
  priv->verify_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
                                               "Verify credentials",
                                               1);
  gtk_widget_set_sensitive (priv->verify_button, FALSE);
  gtk_dialog_add_button (GTK_DIALOG (dialog),
                         GTK_STOCK_CANCEL,
                         GTK_RESPONSE_CANCEL);
}

static void
tweet_auth_dialog_class_init (TweetAuthDialogClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetAuthDialogPrivate));

  gobject_class->constructed = tweet_auth_dialog_constructed;
  gobject_class->dispose = tweet_auth_dialog_dispose;
}

static void
tweet_auth_dialog_init (TweetAuthDialog *dialog)
{
  dialog->priv = TWEET_AUTH_DIALOG_GET_PRIVATE (dialog);

  dialog->priv->config = tweet_config_get_default ();

  dialog->priv->has_email = FALSE;
  dialog->priv->has_password = FALSE;
}

GtkWidget *
tweet_auth_dialog_new (GtkWidget   *parent,
                       const gchar *title)
{
  g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (TWEET_TYPE_AUTH_DIALOG,
                       "title", title,
                       NULL);
}

G_CONST_RETURN gchar *
tweet_auth_dialog_get_username (TweetAuthDialog *dialog)
{
  g_return_val_if_fail (TWEET_IS_AUTH_DIALOG (dialog), NULL);

  return gtk_entry_get_text (GTK_ENTRY (dialog->priv->email_entry));
}

G_CONST_RETURN gchar *
tweet_auth_dialog_get_password (TweetAuthDialog *dialog)
{
  g_return_val_if_fail (TWEET_IS_AUTH_DIALOG (dialog), NULL);

  return gtk_entry_get_text (GTK_ENTRY (dialog->priv->password_entry));
}
