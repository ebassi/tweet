/* tweet-window.c: Main application window
 *
 * This file is part of Tweet.
 * Copyright (C) 2008  Emmanuele Bassi  <ebassi@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
#include <tidy/tidy-stylable.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-animation.h"
#include "tweet-config.h"
#include "tweet-spinner.h"
#include "tweet-status-model.h"
#include "tweet-status-view.h"
#include "tweet-utils.h"
#include "tweet-window.h"

#define CANVAS_WIDTH    350
#define CANVAS_HEIGHT   500
#define CANVAS_PADDING  6

#define WINDOW_WIDTH    (CANVAS_WIDTH + (2 * CANVAS_PADDING))

#define TWEET_WINDOW_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_WINDOW, TweetWindowPrivate))

struct _TweetWindowPrivate
{
  GtkWidget *vbox;
  GtkWidget *entry;
  GtkWidget *canvas;
  GtkWidget *send_button;
  GtkWidget *counter;

  ClutterActor *spinner;
  ClutterActor *status_view;
  ClutterActor *scroll;

  TwitterClient *client;

  TweetConfig *config;
  TweetStatusModel *status_model;

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
      twitter_client_end_session (priv->client);
      g_object_unref (priv->client);
      priv->client = NULL;
    }

  if (priv->status_model)
    {
      g_object_unref (priv->status_model);
      priv->status_model = NULL;
    }

  G_OBJECT_CLASS (tweet_window_parent_class)->dispose (gobject);
}

static void
on_status_received (TwitterClient *client,
                    TwitterStatus *status,
                    const GError  *error,
                    TweetWindow   *window)
{
  TweetWindowPrivate *priv = window->priv;

  if (error)
    {
      tweet_spinner_stop (TWEET_SPINNER (priv->spinner));
      tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                           "opacity", tweet_interval_new (G_TYPE_UCHAR, 127, 0),
                           NULL);
      g_warning ("Unable to retrieve status from Twitter: %s", error->message);
    }
  else
    tweet_status_model_prepend_status (window->priv->status_model, status);
}

static void
on_timeline_complete (TwitterClient *client,
                      TweetWindow   *window)
{
  TweetWindowPrivate *priv = window->priv;

  tweet_spinner_stop (TWEET_SPINNER (priv->spinner));
  tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 127, 0),
                       NULL);
}

static void
on_entry_changed (GtkEntry *entry,
                  TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;
  const gchar *status_text = gtk_entry_get_text (entry);
  const gchar *color;
  gchar *count_text;

  if (strlen (status_text) == 0)
    {
      gtk_widget_set_sensitive (priv->send_button, FALSE);
      color = "green";
    }
  else
    {
      gtk_widget_set_sensitive (priv->send_button, TRUE);

      if (strlen (status_text) < 140)
        color = "green";
      else
        color = "red";
    }

  count_text = g_strdup_printf ("<span color='%s'>%d</span>",
                                color,
                                strlen (status_text));

  gtk_label_set_text (GTK_LABEL (priv->counter), count_text);
  gtk_label_set_use_markup (GTK_LABEL (priv->counter), TRUE);

  g_free (count_text);
}

static void
on_entry_activate (GtkEntry *entry,
                   TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;
  const gchar *text;
  gchar *status_text;

  text = gtk_entry_get_text (entry);
  if (!text || *text == '\0')
    return;

  status_text = g_markup_escape_text (gtk_entry_get_text (entry), -1);

  twitter_client_add_status (priv->client, status_text);

  gtk_entry_set_text (entry, "");

  g_free (status_text);
}

static gboolean
refresh_timeout (gpointer data)
{
  TweetWindow *window = data;
  TweetWindowPrivate *priv = window->priv;

  tidy_list_view_set_model (TIDY_LIST_VIEW (priv->status_view), NULL);
  g_object_unref (priv->status_model);

  priv->status_model = TWEET_STATUS_MODEL (tweet_status_model_new ());
  tidy_list_view_set_model (TIDY_LIST_VIEW (priv->status_view),
                            CLUTTER_MODEL (priv->status_model));

  clutter_actor_show (window->priv->spinner);
  tweet_spinner_start (TWEET_SPINNER (window->priv->spinner));
  tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 0, 127),
                       NULL);

  twitter_client_get_user_timeline (window->priv->client, NULL, 0, NULL);

  return TRUE;
}

static void
tweet_window_constructed (GObject *gobject)
{
  TweetWindow *window = TWEET_WINDOW (gobject);
  TweetWindowPrivate *priv = window->priv;
  ClutterActor *stage;
  ClutterActor *view;
  ClutterActor *img;
  ClutterColor stage_color = { 0, 0, 0, 255 };

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  view = tweet_status_view_new (priv->status_model);
  priv->scroll = tidy_finger_scroll_new (TIDY_FINGER_SCROLL_MODE_KINETIC);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->scroll), view);
  clutter_actor_show (view);
  clutter_actor_set_reactive (view, TRUE);
  priv->status_view = view;

  clutter_actor_set_size (priv->scroll, CANVAS_WIDTH, CANVAS_HEIGHT);
  clutter_actor_set_position (priv->scroll, CANVAS_PADDING, CANVAS_PADDING);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->scroll);
  clutter_actor_set_reactive (priv->scroll, TRUE);
  clutter_actor_show (priv->scroll);

  img = tweet_texture_new_from_stock (GTK_WIDGET (window),
                                      GTK_STOCK_REFRESH,
                                      GTK_ICON_SIZE_DIALOG);
  if (!img)
    g_warning ("Unable to load the `%s' stock icon", GTK_STOCK_REFRESH);

  priv->spinner = tweet_spinner_new ();
  tweet_spinner_set_image (TWEET_SPINNER (priv->spinner), img);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->spinner);
  clutter_actor_set_size (priv->spinner, 128, 128);
  clutter_actor_set_anchor_point (priv->spinner, 64, 64);
  clutter_actor_set_position (priv->spinner,
                              WINDOW_WIDTH / 2,
                              CANVAS_HEIGHT / 2);
  clutter_actor_show (priv->spinner);
  tweet_spinner_start (TWEET_SPINNER (priv->spinner));
  tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 0, 127),
                       NULL);

  clutter_actor_show (stage);
  gtk_widget_show (GTK_WIDGET (window));

  twitter_client_get_user_timeline (priv->client, NULL, 0, NULL);

  priv->refresh_id =
    g_timeout_add_seconds (tweet_config_get_refresh_time (priv->config),
                           refresh_timeout,
                           window);
}

static void
tweet_window_style_set (GtkWidget *widget,
                        GtkStyle  *old_style)
{
  TweetWindowPrivate *priv = TWEET_WINDOW (widget)->priv;
  ClutterColor active_color = { 0, };
  ClutterColor text_color = { 0, };
  ClutterColor bg_color = { 0, };
  gchar *font_name;

  tweet_widget_get_base_color (widget, GTK_STATE_SELECTED, &active_color);
  tweet_widget_get_text_color (widget, GTK_STATE_NORMAL, &text_color);
  tweet_widget_get_bg_color (widget, GTK_STATE_NORMAL, &bg_color);

  font_name = pango_font_description_to_string (widget->style->font_desc);

  tidy_stylable_set (TIDY_STYLABLE (priv->scroll),
                     "active-color", &active_color,
                     "bg-color", &bg_color,
                     NULL);
  tidy_stylable_set (TIDY_STYLABLE (priv->status_view),
                     "active-color", &active_color,
                     "bg-color", &bg_color,
                     "text-color", &text_color,
                     "font-name", font_name,
                     NULL);

  g_free (font_name);
}

static void
tweet_window_class_init (TweetWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetWindowPrivate));

  gobject_class->dispose = tweet_window_dispose;
  gobject_class->constructed = tweet_window_constructed;

  widget_class->style_set = tweet_window_style_set;
}

static void
tweet_window_init (TweetWindow *window)
{
  TweetWindowPrivate *priv;
  GtkWidget *hbox, *button;

  GTK_WINDOW (window)->type = GTK_WINDOW_TOPLEVEL;
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_window_set_title (GTK_WINDOW (window), "Tweet");
  gtk_widget_set_size_request (GTK_WIDGET (window), WINDOW_WIDTH, -1);

  window->priv = priv = TWEET_WINDOW_GET_PRIVATE (window);

  priv->vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), priv->vbox);
  gtk_widget_show (priv->vbox);

  priv->canvas = gtk_clutter_embed_new ();
  gtk_widget_set_size_request (priv->canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
  gtk_container_add (GTK_CONTAINER (priv->vbox), priv->canvas);
  gtk_widget_show (priv->canvas);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_end (GTK_BOX (priv->vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  priv->entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 0);
  gtk_widget_set_tooltip_text (priv->entry, "Update your status");
  gtk_widget_show (priv->entry);
  g_signal_connect (priv->entry,
                    "activate", G_CALLBACK (on_entry_activate),
                    window);
  g_signal_connect (priv->entry,
                    "changed", G_CALLBACK (on_entry_changed),
                    window);

  priv->counter = gtk_label_new ("<span color='green'>0</span>");
  gtk_label_set_use_markup (GTK_LABEL (priv->counter), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), priv->counter, FALSE, FALSE, 0);
  gtk_widget_show (priv->counter);

  button = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_stock (GTK_STOCK_JUMP_TO,
                                                  GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_show (button);
  g_signal_connect_swapped (button,
                            "clicked", G_CALLBACK (gtk_widget_activate),
                            priv->entry);
  priv->send_button = button;

  priv->status_model = TWEET_STATUS_MODEL (tweet_status_model_new ());

  priv->config = tweet_config_get_default ();
  priv->client = twitter_client_new_for_user (tweet_config_get_username (priv->config),
                                              tweet_config_get_password (priv->config));
  g_signal_connect (priv->client,
                    "status-received", G_CALLBACK (on_status_received),
                    window);
  g_signal_connect (priv->client,
                    "timeline-complete", G_CALLBACK (on_timeline_complete),
                    window);
}

GtkWidget *
tweet_window_new (void)
{
  return g_object_new (TWEET_TYPE_WINDOW, NULL);
}
