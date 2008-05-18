#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include <gtk/gtk.h>

#include <clutter/clutter.h>
#include <clutter-gtk/gtk-clutter-embed.h>

#include <tidy/tidy-finger-scroll.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-config.h"
#include "tweet-status-model.h"
#include "tweet-status-view.h"
#include "tweet-window.h"

#define TWEET_WINDOW_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_WINDOW, TweetWindowPrivate))

struct _TweetWindowPrivate
{
  GtkWidget *vbox;
  GtkWidget *entry;
  GtkWidget *canvas;

  TwitterClient *client;

  TweetConfig *config;
  TweetStatusModel *model;

  guint refresh_id;
};

G_DEFINE_TYPE (TweetWindow, tweet_window, GTK_TYPE_WINDOW);

static void
tweet_window_dispose (GObject *gobject)
{
  TweetWindowPrivate *priv = TWEET_WINDOW (gobject)->priv;

  if (priv->refresh_id)
    {
      g_source_remove (priv->refresh_id);
      priv->refresh_id = 0;
    }

  if (priv->client)
    {
      g_object_unref (priv->client);
      priv->client = NULL;
    }

  if (priv->model)
    {
      g_object_unref (priv->model);
      priv->model = NULL;
    }

  G_OBJECT_CLASS (tweet_window_parent_class)->dispose (gobject);
}

static void
on_status_received (TwitterClient *client,
                    TwitterStatus *status,
                    const GError  *error,
                    gpointer       user_data)
{
  TweetWindow *window = user_data;

  if (error)
    {
      g_warning ("Unable to retrieve status from Twitter: %s", error->message);
      return;
    }

  tweet_status_model_append_status (window->priv->model, status);
}

static void
on_entry_activate (GtkEntry *entry,
                   TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;
  gchar *status_text;

  status_text = g_markup_escape_text (gtk_entry_get_text (entry), -1);
  if (!status_text || strlen (status_text) == 0)
    {
      g_free (status_text);
      return;
    }

  twitter_client_add_status (priv->client, status_text);

  gtk_entry_set_text (entry, "");

  g_free (status_text);
}

static gboolean
refresh_timeout (gpointer data)
{
  TweetWindow *window = data;

  twitter_client_get_user_timeline (window->priv->client, NULL, 0, NULL);

  return FALSE;
}

static void
tweet_window_constructed (GObject *gobject)
{
  TweetWindow *window = TWEET_WINDOW (gobject);
  TweetWindowPrivate *priv = window->priv;
  ClutterActor *stage;
  ClutterActor *view;
  ClutterActor *scroll;
  ClutterColor stage_color = { 0, 0, 0, 255 };

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  view = tweet_status_view_new (priv->model);
  scroll = tidy_finger_scroll_new (TIDY_FINGER_SCROLL_MODE_KINETIC);
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll), view);
  clutter_actor_show (view);

  clutter_actor_set_size (scroll, 350, 500);
  clutter_actor_set_position (scroll, 6, 6);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), scroll);
  clutter_actor_show (scroll);

  clutter_actor_show (stage);
  gtk_widget_show (GTK_WIDGET (window));

  twitter_client_get_user_timeline (priv->client, NULL, 0, NULL);
  priv->refresh_id = g_timeout_add_seconds (tweet_config_get_refresh_time (priv->config),
                                            refresh_timeout,
                                            window);
}

static void
tweet_window_class_init (TweetWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetWindowPrivate));

  gobject_class->dispose = tweet_window_dispose;
  gobject_class->constructed = tweet_window_constructed;
}

static void
tweet_window_init (TweetWindow *window)
{
  TweetWindowPrivate *priv;
  GtkWidget *hbox, *button;

  GTK_WINDOW (window)->type = GTK_WINDOW_TOPLEVEL;
  gtk_window_set_default_size (GTK_WINDOW (window), 364, 500);
  gtk_window_set_title (GTK_WINDOW (window), "Tweet");

  window->priv = priv = TWEET_WINDOW_GET_PRIVATE (window);

  priv->vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), priv->vbox);
  gtk_widget_show (priv->vbox);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (priv->vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  priv->entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 0);
  gtk_widget_show (priv->entry);
  g_signal_connect (priv->entry, "activate",
                    G_CALLBACK (on_entry_activate),
                    window);

  button = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_stock (GTK_STOCK_JUMP_TO,
                                                  GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_activate), priv->entry);

  priv->canvas = gtk_clutter_embed_new ();
  gtk_container_add (GTK_CONTAINER (priv->vbox), priv->canvas);
  gtk_widget_show (priv->canvas);

  priv->model = TWEET_STATUS_MODEL (tweet_status_model_new ());

  priv->config = tweet_config_get_default ();
  priv->client = twitter_client_new_for_user (tweet_config_get_username (priv->config),
                                              tweet_config_get_password (priv->config));
  g_signal_connect (priv->client,
                    "status-received", G_CALLBACK (on_status_received),
                    window);
}

GtkWidget *
tweet_window_new (void)
{
  return g_object_new (TWEET_TYPE_WINDOW, NULL);
}
