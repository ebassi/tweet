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

#include <glib/gi18n.h>

#include <glib-object.h>

#ifdef HAVE_NM_GLIB
#include <libnm_glib.h>
#endif

#include <gtk/gtk.h>

#include <clutter/clutter.h>
#include <clutter-gtk/gtk-clutter-embed.h>

#include <tidy/tidy-finger-scroll.h>
#include <tidy/tidy-stylable.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-animation.h"
#include "tweet-config.h"
#include "tweet-preferences.h"
#include "tweet-spinner.h"
#include "tweet-status-info.h"
#include "tweet-status-model.h"
#include "tweet-status-view.h"
#include "tweet-utils.h"
#include "tweet-window.h"

#define CANVAS_WIDTH    350
#define CANVAS_HEIGHT   500
#define CANVAS_PADDING  6

#define WINDOW_WIDTH    (CANVAS_WIDTH + (2 * CANVAS_PADDING))

typedef enum {
  TWEET_WINDOW_RECENT,
  TWEET_WINDOW_REPLIES,
  TWEET_WINDOW_ARCHIVE,
  TWEET_WINDOW_FAVORITES
} TweetWindowMode;

typedef enum {
  TWEET_STATUS_ERROR,
  TWEET_STATUS_RECEIVED,
  TWEET_STATUS_NO_CONNECTION,
  TWEET_STATUS_MESSAGE
} TweetStatusMode;

#define TWEET_WINDOW_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_WINDOW, TweetWindowPrivate))

struct _TweetWindowPrivate
{
  GtkWidget *vbox;
  GtkWidget *menubar;
  GtkWidget *entry;
  GtkWidget *canvas;
  GtkWidget *send_button;
  GtkWidget *counter;

  GtkStatusIcon *status_icon;

  ClutterActor *spinner;
  ClutterActor *status_view;
  ClutterActor *scroll;
  ClutterActor *info;

  GtkUIManager *manager;
  GtkActionGroup *action_group;

  TwitterClient *client;
  TwitterUser *user;

  gint n_status_received;

  GTimeVal last_update;

  TweetConfig *config;
  TweetStatusModel *status_model;

  gint press_x;
  gint press_y;
  gint press_row;
  guint in_press : 1;

  guint refresh_id;

  TweetWindowMode mode;

#ifdef HAVE_NM_GLIB
  libnm_glib_ctx *nm_context;
  guint nm_id;
  libnm_glib_state nm_state;
#endif
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

  if (priv->user)
    {
      g_object_unref (priv->user);
      priv->user = NULL;
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

  if (priv->manager)
    {
      g_object_unref (priv->manager);
      priv->manager = NULL;
    }

  if (priv->status_icon)
    {
      g_object_unref (priv->status_icon);
      priv->status_icon = NULL;
    }

  if (priv->action_group)
    {
      g_object_unref (priv->action_group);
      priv->action_group = NULL;
    }

#ifdef HAVE_NM_GLIB
  if (priv->nm_id)
    {
      libnm_glib_unregister_callback (priv->nm_context, priv->nm_id);
      libnm_glib_shutdown (priv->nm_context);

      priv->nm_id = 0;
      priv->nm_context = NULL;
    }
#endif /* HAVE_NM_GLIB */

  G_OBJECT_CLASS (tweet_window_parent_class)->dispose (gobject);
}

static void
on_status_icon_activate (GtkStatusIcon *icon,
                         TweetWindow   *window)
{
  gtk_window_present (GTK_WINDOW (window));

  gtk_status_icon_set_visible (window->priv->status_icon, FALSE);
}

static void
tweet_window_status_message (TweetWindow     *window,
                             TweetStatusMode  status_mode,
                             const gchar     *format,
                             ...)
{
  TweetWindowPrivate *priv = window->priv;
  va_list args;
  gchar *message;

  if (!priv->status_icon)
    {
      priv->status_icon = gtk_status_icon_new_from_icon_name ("tweet");
      g_signal_connect (priv->status_icon,
                        "activate", G_CALLBACK (on_status_icon_activate),
                        window);
    }


  va_start (args, format);
  message = g_strdup_vprintf (format, args);
  va_end (args);


  switch (status_mode)
    {
    case TWEET_STATUS_MESSAGE:
      break;

    case TWEET_STATUS_ERROR:
      gtk_status_icon_set_from_icon_name (priv->status_icon, "tweet-error");
      gtk_status_icon_set_visible (priv->status_icon, TRUE);
      gtk_status_icon_set_tooltip (priv->status_icon, message);
      break;

    case TWEET_STATUS_NO_CONNECTION:
      gtk_status_icon_set_from_icon_name (priv->status_icon, "tweet-no-connection");
      gtk_status_icon_set_visible (priv->status_icon, TRUE);
      gtk_status_icon_set_tooltip (priv->status_icon, message);
      break;

    case TWEET_STATUS_RECEIVED:
      gtk_status_icon_set_from_icon_name (priv->status_icon, "tweet-new-status");
      gtk_status_icon_set_visible (priv->status_icon, TRUE);
      gtk_status_icon_set_tooltip (priv->status_icon, message);
      break;
    }

  g_free (message);
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

      /* if the content was not modified since the last update,
       * silently ignore the error; Twitter-GLib still emits it
       * so that clients can notify the user anyway
       */
      if (error->domain == TWITTER_ERROR &&
          error->code == TWITTER_ERROR_NOT_MODIFIED)
        {
          g_get_current_time (&priv->last_update);
        }
      else
        tweet_window_status_message (window, TWEET_STATUS_ERROR,
                                     _("Unable to retrieve status from Twitter: %s"),
                                     error->message);

      priv->n_status_received = 0;
    }
  else
    {
      if (!priv->status_model)
        {
          priv->status_model = TWEET_STATUS_MODEL (tweet_status_model_new ());
          tidy_list_view_set_model (TIDY_LIST_VIEW (priv->status_view),
                                    CLUTTER_MODEL (priv->status_model));
        }

      if (tweet_status_model_prepend_status (priv->status_model, status));
        priv->n_status_received += 1;
    }
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

  if (priv->n_status_received > 0)
    {
      gchar *msg;

      msg = g_strdup_printf (ngettext ("Received a new status",
                                       "Received %d new statuses",
                                       priv->n_status_received),
                             priv->n_status_received);

      tweet_window_status_message (window, TWEET_STATUS_RECEIVED, msg);

      g_free (msg);
    }

  priv->n_status_received = 0;

  g_get_current_time (&priv->last_update);
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

  text = gtk_entry_get_text (entry);
  if (!text || *text == '\0')
    return;

  twitter_client_add_status (priv->client, text);

  gtk_entry_set_text (entry, "");
}

static void
on_info_destroy (TweetAnimation *animation,
                 TweetWindow    *window)
{
  clutter_actor_destroy (window->priv->info);
  window->priv->info = NULL;
}

static gboolean
on_info_button_press (ClutterActor       *actor,
                      ClutterButtonEvent *event,
                      TweetWindow        *window)
{
  TweetWindowPrivate *priv = window->priv;
  TweetAnimation *animation;

  animation =
    tweet_actor_animate (actor, TWEET_LINEAR, 250,
                         "opacity", tweet_interval_new (G_TYPE_UCHAR, 224, 0),
                         NULL);
  g_signal_connect (animation,
                    "completed", G_CALLBACK (on_info_destroy),
                    window);

  clutter_actor_set_reactive (priv->status_view, TRUE);
  tweet_actor_animate (priv->status_view, TWEET_LINEAR, 250,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 128, 255),
                       NULL);

  return TRUE;
}

static void
on_star_clicked (TweetStatusInfo *info,
                 TweetWindow     *window)
{
  TweetWindowPrivate *priv = window->priv;
  TwitterStatus *status;

  status = tweet_status_info_get_status (info);
  if (!status)
    return;

  twitter_client_add_favorite (priv->client, twitter_status_get_id (status));
}

static void
on_reply_clicked (TweetStatusInfo *info,
                  TweetWindow     *window)
{
  TweetWindowPrivate *priv = window->priv;
  TwitterStatus *status;
  TwitterUser *user;
  gchar *reply_to;

  status = tweet_status_info_get_status (info);
  if (!status)
    return;

  user = twitter_status_get_user (status);
  if (!user)
    return;

  reply_to = g_strdup_printf ("@%s ", twitter_user_get_screen_name (user));

  gtk_entry_set_text (GTK_ENTRY (priv->entry), reply_to);
  gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);

  g_free (reply_to);
}

static void
on_icon_clicked (TweetStatusInfo *info,
                 TweetWindow     *window)
{
  TwitterStatus *status;
  TwitterUser *user;
  GdkScreen *screen;
  gint pid;
  GError *error;
  gchar **argv;

  status = tweet_status_info_get_status (info);
  g_assert (TWITTER_IS_STATUS (status));

  user = twitter_status_get_user (status);
  if (!user)
    return;

  if (gtk_widget_has_screen (GTK_WIDGET (window)))
    screen = gtk_widget_get_screen (GTK_WIDGET (window));
  else
    screen = gdk_screen_get_default ();

  argv = g_new (gchar*, 3);
  argv[0] = g_strdup ("xdg-open");
  argv[1] = g_strdup_printf ("http://twitter.com/%s",
                             twitter_user_get_screen_name (user));
  argv[2] = NULL;

  error = NULL;
  gdk_spawn_on_screen (screen,
                       NULL,
                       argv, NULL,
                       G_SPAWN_SEARCH_PATH,
                       NULL, NULL,
                       &pid, &error);
  if (error)
    {
      g_critical ("Unable to launch gnome-open: %s", error->message);
      g_error_free (error);
    }

  g_strfreev (argv);
}

static void
on_status_info_visible (TweetAnimation *animation,
                        TweetWindow    *window)
{
  window->priv->in_press = FALSE;
  clutter_actor_set_reactive (window->priv->info, TRUE);
}

static gboolean
on_status_view_button_press (ClutterActor       *actor,
                             ClutterButtonEvent *event,
                             TweetWindow        *window)
{
  TweetWindowPrivate *priv = window->priv;
  gint row;

  /* this should not happen, but just in case... */
  if (priv->info)
    return FALSE;

  row = tidy_list_view_get_row_at_pos (TIDY_LIST_VIEW (actor),
                                       event->x,
                                       event->y);

  if (row >= 0)
    {
      priv->in_press = TRUE;

      priv->press_x = event->x;
      priv->press_y = event->y;
      priv->press_row = row;
    }

  return FALSE;
}

static gboolean
on_status_view_button_release (ClutterActor       *actor,
                               ClutterButtonEvent *event,
                               TweetWindow        *window)
{
  TweetWindowPrivate *priv = window->priv;

/* in case of a crappy touchscreen */
#define JITTER  5

  if (priv->in_press)
    {
      TweetAnimation *animation;
      TwitterStatus *status;
      ClutterModelIter *iter;
      ClutterGeometry geometry = { 0, };
      ClutterActor *stage;
      ClutterColor info_color = { 255, 255, 255, 255 };

      priv->in_press = FALSE;

      if (abs (priv->press_y - event->y) > JITTER)
        {
          priv->in_press = FALSE;
          return FALSE;
        }

      iter = clutter_model_get_iter_at_row (CLUTTER_MODEL (priv->status_model),
                                            priv->press_row);
      if (!iter)
        {
          priv->in_press = FALSE;
          return FALSE;
        }

      status = tweet_status_model_get_status (priv->status_model, iter);
      if (!status)
        {
          g_object_unref (iter);
          priv->in_press = FALSE;
          return FALSE;
        }

      tweet_status_view_get_cell_geometry (TWEET_STATUS_VIEW (priv->status_view),
                                           priv->press_row,
                                           TRUE,
                                           &geometry);

      stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));

      priv->info = tweet_status_info_new (status);
      tweet_overlay_set_color (TWEET_OVERLAY (priv->info), &info_color);
      g_signal_connect (priv->info,
                        "button-press-event", G_CALLBACK (on_info_button_press),
                        window);
      g_signal_connect (priv->info,
                        "star-clicked", G_CALLBACK (on_star_clicked),
                        window);
      g_signal_connect (priv->info,
                        "reply-clicked", G_CALLBACK (on_reply_clicked),
                        window);
      g_signal_connect (priv->info,
                        "icon-clicked", G_CALLBACK (on_icon_clicked),
                        window);
                                
      clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->info);
      clutter_actor_set_position (priv->info,
                                  geometry.x + CANVAS_PADDING,
                                  geometry.y + CANVAS_PADDING);
      clutter_actor_set_size (priv->info, geometry.width - CANVAS_PADDING, 16);
      clutter_actor_set_opacity (priv->info, 0);
      clutter_actor_set_reactive (priv->info, FALSE);
      clutter_actor_show (priv->info);

      /* the status info is non-reactive until it has
       * been fully "unrolled" by the animation
       */
      animation =
        tweet_actor_animate (priv->info, TWEET_LINEAR, 250,
                             "y", tweet_interval_new (G_TYPE_INT, geometry.y + CANVAS_PADDING, 100 + CANVAS_PADDING),
                             "height", tweet_interval_new (G_TYPE_INT, 16, (CANVAS_HEIGHT - (100 * 2))),
                             "opacity", tweet_interval_new (G_TYPE_UCHAR, 0, 224),
                             NULL);
      g_signal_connect (animation,
                        "completed", G_CALLBACK (on_status_info_visible),
                        window);

      /* set the status view as not reactive to avoid opening
       * the status info on double tap
       */
      clutter_actor_set_reactive (priv->status_view, FALSE);
      tweet_actor_animate (priv->status_view, TWEET_LINEAR, 250,
                           "opacity", tweet_interval_new (G_TYPE_UCHAR, 255, 128),
                           NULL);

      g_object_unref (status);
      g_object_unref (iter);
    }

#undef JITTER

  return FALSE;
}

static inline void
tweet_window_clear (TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;

  tidy_list_view_set_model (TIDY_LIST_VIEW (priv->status_view), NULL);
  g_object_unref (priv->status_model);
  priv->status_model = NULL;
}

static inline void
tweet_window_refresh (TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;

  clutter_actor_show (priv->spinner);
  tweet_spinner_start (TWEET_SPINNER (priv->spinner));
  tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 0, 127),
                       NULL);

  /* check for the user */
  if (!priv->user)
    twitter_client_show_user_from_email (priv->client,
                                         tweet_config_get_username (priv->config));

  switch (priv->mode)
    {
    case TWEET_WINDOW_RECENT:
      priv->n_status_received = 0;
      twitter_client_get_friends_timeline (priv->client,
                                           NULL,
                                           priv->last_update.tv_sec);
      break;

    case TWEET_WINDOW_REPLIES:
      twitter_client_get_replies (priv->client);
      break;

    case TWEET_WINDOW_ARCHIVE:
      twitter_client_get_user_timeline (priv->client,
                                        NULL,
                                        0,
                                        priv->last_update.tv_sec);
      break;

    case TWEET_WINDOW_FAVORITES:
      twitter_client_get_favorites (priv->client, NULL, 0);
      break;
    }

}

static gboolean
refresh_timeout (gpointer data)
{
  TweetWindow *window = data;

  tweet_window_refresh (window);

  return TRUE;
}

static void
on_user_received (TwitterClient *client,
                  TwitterUser   *user,
                  const GError  *error,
                  TweetWindow   *window)
{
  TweetWindowPrivate *priv = window->priv;

  if (error)
    {
      priv->user = NULL;

      tweet_window_status_message (window, TWEET_STATUS_ERROR,
                                   _("Unable to retrieve user `%s': %s"),
                                   tweet_config_get_username (priv->config),
                                   error->message);
      return;
    }

  if (priv->user)
    g_object_unref (priv->user);

  /* keep a reference on ourselves */
  priv->user = g_object_ref (user);
}

#ifdef HAVE_NM_GLIB
static void
nm_context_callback (libnm_glib_ctx *libnm_ctx,
                     gpointer        user_data)
{
  TweetWindow *window = user_data;
  TweetWindowPrivate *priv = window->priv;
  libnm_glib_state nm_state;
  gint refresh_time;

  nm_state = libnm_glib_get_network_state (libnm_ctx);

  if (nm_state == priv->nm_state)
    return;

  switch (nm_state)
    {
    case LIBNM_ACTIVE_NETWORK_CONNECTION:
      refresh_time = tweet_config_get_refresh_time (priv->config);

      if (refresh_time > 0)
        {
          if (priv->refresh_id)
            break;

          tweet_window_refresh (window);
          priv->refresh_id = g_timeout_add_seconds (refresh_time,
                                                    refresh_timeout,
                                                    window);
        }
      else
        tweet_window_refresh (window);

      break;

    case LIBNM_NO_DBUS:
    case LIBNM_NO_NETWORKMANAGER:
      g_critical ("No NetworkManager running");
      break;

    case LIBNM_NO_NETWORK_CONNECTION:
      tweet_window_status_message (window, TWEET_STATUS_NO_CONNECTION,
                                   _("No network connection available"));
      g_source_remove (priv->refresh_id);
      priv->refresh_id = 0;
      tweet_window_clear (window);
      break;

    case LIBNM_INVALID_CONTEXT:
      g_critical ("Invalid NetworkManager-GLib context");
      break;
    }

  priv->nm_state = nm_state;
}
#endif /* HAVE_NM_GLIB */

static void
tweet_window_constructed (GObject *gobject)
{
  TweetWindow *window = TWEET_WINDOW (gobject);
  TweetWindowPrivate *priv = window->priv;
  ClutterActor *stage;
  ClutterActor *img;
#ifdef HAVE_NM_GLIB
  libnm_glib_state nm_state;
#endif /* HAVE_NM_GLIB */

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));

  img = tweet_texture_new_from_stock (GTK_WIDGET (window),
                                      GTK_STOCK_REFRESH,
                                      GTK_ICON_SIZE_DIALOG);
  if (!img)
    g_critical ("Unable to load the `%s' stock icon", GTK_STOCK_REFRESH);

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

  gtk_widget_show_all (GTK_WIDGET (window));

#ifdef HAVE_NM_GLIB
  priv->nm_context = libnm_glib_init ();

  nm_state = libnm_glib_get_network_state (priv->nm_context);
  if (nm_state == LIBNM_ACTIVE_NETWORK_CONNECTION)
    {
      const gchar *email_address;
      gint refresh_time;

      g_signal_connect (priv->client,
                        "user-received", G_CALLBACK (on_user_received),
                        window);

      email_address = tweet_config_get_username (priv->config);
      twitter_client_show_user_from_email (priv->client, email_address);

      twitter_client_get_friends_timeline (priv->client, NULL, 0);

      refresh_time = tweet_config_get_refresh_time (priv->config);
      if (refresh_time > 0)
        priv->refresh_id = g_timeout_add_seconds (refresh_time,
                                                  refresh_timeout,
                                                  window);
    }
  else
    {
      tweet_window_status_message (window, TWEET_STATUS_NO_CONNECTION,
                                   _("No network connection available"));

      tweet_spinner_stop (TWEET_SPINNER (priv->spinner));
      tweet_actor_animate (priv->spinner, TWEET_LINEAR, 500,
                           "opacity", tweet_interval_new (G_TYPE_UCHAR, 127, 0),
                           NULL);
    }

  priv->nm_state = nm_state;
  priv->nm_id = libnm_glib_register_callback (priv->nm_context,
                                              nm_context_callback,
                                              window,
                                              NULL);
#else
  {
    const gchar *email_address;
    gint refresh_time;

    g_signal_connect (priv->client,
                      "user-received", G_CALLBACK (on_user_received),
                      window);

    email_address = tweet_config_get_username (priv->config);
    twitter_client_show_user_from_email (priv->client, email_address);

    twitter_client_get_friends_timeline (priv->client, NULL, 0);

    refresh_time = tweet_config_get_refresh_time (priv->config);
    if (refresh_time > 0)
      priv->refresh_id = g_timeout_add_seconds (refresh_time,
                                                refresh_timeout,
                                                window);
  }
#endif /* HAVE_NM_GLIB */
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

  if (tweet_config_get_use_gtk_bg (priv->config))
    {
      ClutterActor *stage;

      stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));
      clutter_stage_set_color (CLUTTER_STAGE (stage), &bg_color);
    }

  g_free (font_name);
}

static void
tweet_window_cmd_quit (GtkAction   *action,
                       TweetWindow *window)
{
  gtk_widget_destroy (GTK_WIDGET (window));
}

static void
tweet_window_cmd_preferences (GtkAction   *action,
                              TweetWindow *window)
{
  tweet_show_preferences_dialog (GTK_WINDOW (window),
                                 _("Preferences"),
                                 window->priv->config);
}

static void
tweet_window_cmd_view_recent (GtkAction   *action,
                              TweetWindow *window)
{
  if (window->priv->mode == TWEET_WINDOW_RECENT)
    return;

  window->priv->mode = TWEET_WINDOW_RECENT;
  window->priv->last_update.tv_sec = 0;

  tweet_window_clear (window);
  tweet_window_refresh (window);
}

static void
tweet_window_cmd_view_replies (GtkAction   *action,
                               TweetWindow *window)
{
  if (window->priv->mode == TWEET_WINDOW_REPLIES)
    return;

  window->priv->mode = TWEET_WINDOW_REPLIES;
  window->priv->last_update.tv_sec = 0;

  tweet_window_clear (window);
  tweet_window_refresh (window);
}

static void
tweet_window_cmd_view_archive (GtkAction   *action,
                               TweetWindow *window)
{
  if (window->priv->mode == TWEET_WINDOW_ARCHIVE)
    return;

  window->priv->mode = TWEET_WINDOW_ARCHIVE;
  window->priv->last_update.tv_sec = 0;

  tweet_window_clear (window);
  tweet_window_refresh (window);
}

static void
tweet_window_cmd_view_favorites (GtkAction   *action,
                                 TweetWindow *window)
{
  if (window->priv->mode == TWEET_WINDOW_FAVORITES)
    return;

  window->priv->mode = TWEET_WINDOW_FAVORITES;
  window->priv->last_update.tv_sec = 0;

  tweet_window_clear (window);
  tweet_window_refresh (window);
}

static void
tweet_window_cmd_view_reload (GtkAction   *action,
                              TweetWindow *window)
{
  TweetWindowPrivate *priv = window->priv;

  if (priv->refresh_id)
    {
      g_source_remove (priv->refresh_id);
      priv->refresh_id = 0;
    }

  tweet_window_refresh (window);

  priv->refresh_id =
    g_timeout_add_seconds (tweet_config_get_refresh_time (priv->config),
                           refresh_timeout,
                           window);
}

static void
about_url_hook (GtkAboutDialog *dialog,
                const gchar    *link_,
                gpointer        user_data)
{
  GdkScreen *screen;
  gint pid;
  GError *error;
  gchar **argv;

  if (gtk_widget_has_screen (GTK_WIDGET (dialog)))
    screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
  else
    screen = gdk_screen_get_default ();

  argv = g_new (gchar*, 3);
  argv[0] = g_strdup ("xdg-open");
  argv[1] = g_strdup (link_);
  argv[2] = NULL;

  error = NULL;
  gdk_spawn_on_screen (screen,
                       NULL,
                       argv, NULL,
                       G_SPAWN_SEARCH_PATH,
                       NULL, NULL,
                       &pid, &error);
  if (error)
    {
      g_critical ("Unable to launch gnome-open: %s", error->message);
      g_error_free (error);
    }

  g_strfreev (argv);
}

static void
tweet_window_cmd_help_about (GtkAction   *action,
                             TweetWindow *window)
{
  const gchar *authors[] = {
    "Emmanuele Bassi <ebassi@gnome.org>",
    NULL
  };

  const gchar *artists[] = {
    "Ulisse Perusin <uli.peru@gmail.com>",
    NULL
  };

  const gchar *translator_credits = _("translator-credits");
  const gchar *copyright = "Copyright \xc2\xa9 2008 Emmanuele Bassi";

  const gchar *license_text =
    _("This program is free software: you can redistribute it and/or "
      "modify it under the terms of the GNU General Public License as "
      "published by the Free Software Foundation, either version 3 of "
      "the License, or (at your option) any later version.");

  gtk_about_dialog_set_url_hook (about_url_hook, NULL, NULL);

  gtk_show_about_dialog (GTK_WINDOW (window),
                         "program-name", "Tweet",
                         "title", _("About Tweet"),
                         "comments", _("Twitter desktop client"),
                         "logo-icon-name", "tweet",
                         "version", VERSION,
                         "copyright", copyright,
                         "authors", authors,
                         "artists", artists,
                         "translator-credits", translator_credits,
                         "website", "http://live.gnome.org/Tweet",
                         "license", license_text,
                         "wrap-license", TRUE,
                         NULL);
}

static gboolean
on_canvas_focus_in (GtkWidget *widget,
                    GdkEventFocus *event,
                    TweetWindow   *window)
{
  GtkClutterEmbed *embed = GTK_CLUTTER_EMBED (widget);
  ClutterActor *stage = gtk_clutter_embed_get_stage (embed);

  gtk_widget_queue_draw (widget);

  clutter_stage_set_key_focus (CLUTTER_STAGE (stage),
                               window->priv->status_view);

  gtk_widget_grab_focus (widget);

  return FALSE;
}

static gboolean
on_canvas_focus_out (GtkWidget *widget,
                     GdkEventFocus *event,
                     TweetWindow   *window)
{
  GtkClutterEmbed *embed = GTK_CLUTTER_EMBED (widget);
  ClutterActor *stage = gtk_clutter_embed_get_stage (embed);

  gtk_widget_queue_draw (widget);

  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), NULL);

  return FALSE;
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

static const GtkActionEntry action_entries[] = {
  { "TweetFileAction", NULL, N_("_File") },
    {
      "TweetQuit", GTK_STOCK_QUIT, NULL, "<control>Q",
      N_("Quit Tweet"),
      G_CALLBACK (tweet_window_cmd_quit)
    },

  { "TweetEditAction", NULL, N_("_Edit") },
    {
      "TweetPreferences", GTK_STOCK_PREFERENCES, NULL, NULL,
      N_("Edit Tweet Preferences"),
      G_CALLBACK (tweet_window_cmd_preferences)
    },

  { "TweetViewAction", NULL, N_("_View") },
    {
      "TweetRecent", NULL, N_("_Recent statuses"), NULL, NULL,
      G_CALLBACK (tweet_window_cmd_view_recent)
    },
    {
      "TweetReplies", NULL, N_("R_eplies"), NULL, NULL,
      G_CALLBACK (tweet_window_cmd_view_replies)
    },
    {
      "TweetFavorites", NULL, N_("_Favorites"), NULL, NULL,
      G_CALLBACK (tweet_window_cmd_view_favorites)
    },
    {
      "TweetArchive", NULL, N_("_Archive"), NULL, NULL,
      G_CALLBACK (tweet_window_cmd_view_archive)
    },
    {
      "TweetReload", GTK_STOCK_REFRESH, N_("_Reload"), "<control>R",
      N_("Display the latest statuses"),
      G_CALLBACK (tweet_window_cmd_view_reload)
    },

  { "TweetHelpAction", NULL, N_("_Help") },
    {
      "TweetAbout", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL,
      G_CALLBACK (tweet_window_cmd_help_about)
    }
};

static void
tweet_window_init (TweetWindow *window)
{
  TweetWindowPrivate *priv;
  GtkWidget *frame, *hbox, *button;
  GtkAccelGroup *accel_group;
  GError *error;
  ClutterActor *stage, *view;
  ClutterColor stage_color = { 0, 0, 0, 255 };

  GTK_WINDOW (window)->type = GTK_WINDOW_TOPLEVEL;
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_window_set_default_size (GTK_WINDOW (window), WINDOW_WIDTH, 600);
  gtk_window_set_title (GTK_WINDOW (window), "Tweet");

  window->priv = priv = TWEET_WINDOW_GET_PRIVATE (window);

  priv->mode = TWEET_WINDOW_RECENT;

  priv->status_model = TWEET_STATUS_MODEL (tweet_status_model_new ());

  priv->config = tweet_config_get_default ();
  priv->client = g_object_new (TWITTER_TYPE_CLIENT,
                               "email", tweet_config_get_username (priv->config),
                               "password", tweet_config_get_password (priv->config),
                               "user-agent", "Tweet",
                               NULL);
  g_signal_connect (priv->client,
                    "status-received", G_CALLBACK (on_status_received),
                    window);
  g_signal_connect (priv->client,
                    "timeline-complete", G_CALLBACK (on_timeline_complete),
                    window);

  priv->vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), priv->vbox);
  gtk_widget_show (priv->vbox);

  priv->action_group = gtk_action_group_new ("TweetActions");
  gtk_action_group_set_translation_domain (priv->action_group, NULL);
  gtk_action_group_add_actions (priv->action_group,
                                action_entries,
                                G_N_ELEMENTS (action_entries),
                                window);

  priv->manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (priv->manager,
                                      priv->action_group,
                                      0);

  accel_group = gtk_ui_manager_get_accel_group (priv->manager);
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

  error = NULL;
  if (!gtk_ui_manager_add_ui_from_file (priv->manager,
                                        PKGDATADIR G_DIR_SEPARATOR_S "tweet.ui",
                                        &error))
    {
     g_critical ("Building menus failed: %s", error->message);
     g_error_free (error);
    }
  else
    {
      priv->menubar = gtk_ui_manager_get_widget (priv->manager, "/TweetMenubar");
      gtk_box_pack_start (GTK_BOX (priv->vbox), priv->menubar, FALSE, FALSE, 0);
      gtk_widget_show (priv->menubar);
    }

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (priv->vbox), frame);
  gtk_widget_show (frame);

  priv->canvas = gtk_clutter_embed_new ();
  g_signal_connect (priv->canvas,
                    "focus-in-event", G_CALLBACK (on_canvas_focus_in),
                    window);
  g_signal_connect (priv->canvas,
                    "focus-out-event", G_CALLBACK (on_canvas_focus_out),
                    window);
  GTK_WIDGET_SET_FLAGS (priv->canvas, GTK_CAN_FOCUS);

  gtk_widget_set_size_request (priv->canvas,
                               CANVAS_WIDTH + CANVAS_PADDING,
                               CANVAS_HEIGHT + CANVAS_PADDING);
  gtk_container_add (GTK_CONTAINER (frame), priv->canvas);

  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->canvas));
  gtk_widget_set_size_request (priv->canvas,
                               CANVAS_WIDTH + CANVAS_PADDING,
                               CANVAS_HEIGHT + CANVAS_PADDING);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  clutter_actor_set_size (stage,
                          CANVAS_WIDTH + CANVAS_PADDING,
                          CANVAS_HEIGHT + CANVAS_PADDING);

  view = tweet_status_view_new (priv->status_model);
  g_signal_connect (view,
                    "button-press-event", G_CALLBACK (on_status_view_button_press),
                    window);
  g_signal_connect (view,
                    "button-release-event", G_CALLBACK (on_status_view_button_release),
                    window);
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
                        gtk_image_new_from_icon_name ("document-send",
                                                      GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  gtk_widget_show (button);
  g_signal_connect_swapped (button,
                            "clicked", G_CALLBACK (gtk_widget_activate),
                            priv->entry);
  priv->send_button = button;

  gtk_widget_realize (priv->canvas);
}

GtkWidget *
tweet_window_new (void)
{
  return g_object_new (TWEET_TYPE_WINDOW, NULL);
}
