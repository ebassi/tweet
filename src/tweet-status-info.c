/* tweet-status-info.c: Expanded view for a TwitterStatus
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

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <cairo/cairo.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>

#include <tidy/tidy-actor.h>
#include <tidy/tidy-stylable.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-animation.h"
#include "tweet-status-info.h"
#include "tweet-utils.h"

#define ICON_WIDTH      64
#define ICON_HEIGHT     64

#define V_PADDING       6
#define H_PADDING       12

#define MAX_WIDTH       344

#define ICON_Y          (V_PADDING)

#define TEXT_X          (H_PADDING)
#define TEXT_Y          (ICON_Y + ICON_HEIGHT + (2 * V_PADDING))
#define TEXT_WIDTH      (MAX_WIDTH - (2 * H_PADDING))

#define BUTTON_SIZE     32

#define BG_ROUND_RADIUS 24

enum
{
  PROP_0,

  PROP_STATUS
};

enum
{
  STAR_CLICKED,
  REPLY_CLICKED,

  LAST_SIGNAL
};

static guint info_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (TweetStatusInfo, tweet_status_info, TWEET_TYPE_OVERLAY);

static void
tweet_status_info_dispose (GObject *gobject)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (gobject);

  if (info->escape_re)
    {
      g_regex_unref (info->escape_re);
      info->escape_re = NULL;
    }

  if (info->status)
    {
      g_object_unref (info->status);
      info->status = NULL;
    }

  G_OBJECT_CLASS (tweet_status_info_parent_class)->dispose (gobject);
}

static void
tweet_status_info_set_property (GObject      *gobject,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (gobject);

  switch (prop_id)
    {
    case PROP_STATUS:
      info->status = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_status_info_get_property (GObject    *gobject,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (gobject);

  switch (prop_id)
    {
    case PROP_STATUS:
      g_value_set_object (value, info->status);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static gboolean
on_button_enter (ClutterActor         *actor,
                 ClutterCrossingEvent *event,
                 TweetStatusInfo      *info)
{
  gint x_pos;

  if (actor == info->star_button)
    tweet_texture_set_from_icon_name (CLUTTER_TEXTURE (actor), NULL,
                                      "status-favorite-set",
                                      -1);

  if (actor == info->reply_button)
    tweet_texture_set_from_icon_name (CLUTTER_TEXTURE (actor), NULL,
                                      "reply-to-status-set",
                                      -1);

  clutter_label_set_text (CLUTTER_LABEL (info->button_tip),
                          g_object_get_data (G_OBJECT (actor), "button-tip"));

  x_pos = clutter_actor_get_x (actor);
  if (x_pos > (MAX_WIDTH / 2))
    x_pos = clutter_actor_get_x (actor)
          - clutter_actor_get_width (info->button_tip)
          - H_PADDING;
  else
    x_pos = clutter_actor_get_x (actor)
          + BUTTON_SIZE
          + H_PADDING;

  clutter_actor_set_x (info->button_tip, x_pos);
  tweet_actor_animate (info->button_tip, TWEET_LINEAR, 150,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 0, 255),
                       NULL);

  return FALSE;
}

static gboolean
on_button_leave (ClutterActor         *actor,
                 ClutterCrossingEvent *event,
                 TweetStatusInfo      *info)
{
  if (actor == info->star_button)
    tweet_texture_set_from_icon_name (CLUTTER_TEXTURE (actor), NULL,
                                      "favorite-status",
                                      -1);

  if (actor == info->reply_button)
    tweet_texture_set_from_icon_name (CLUTTER_TEXTURE (actor), NULL,
                                      "reply-to-status",
                                      -1);

  tweet_actor_animate (info->button_tip, TWEET_LINEAR, 150,
                       "opacity", tweet_interval_new (G_TYPE_UCHAR, 255, 0),
                       NULL);

  return FALSE;
}

static gboolean
on_button_press (ClutterActor       *actor,
                 ClutterButtonEvent *event,
                 TweetStatusInfo    *info)
{
  if (actor == info->star_button)
    g_signal_emit (info, info_signals[STAR_CLICKED], 0);
  else if (actor == info->reply_button)
    g_signal_emit (info, info_signals[REPLY_CLICKED], 0);
  else
    return FALSE;

  return TRUE;
}

static void
tweet_status_info_constructed (GObject *gobject)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (gobject);
  ClutterColor text_color = { 0, 0, 0, 255 };
  TwitterUser *user;
  gchar *font_name, *text, *created_at, *escaped;
  GTimeVal timeval = { 0, };
  GdkPixbuf *pixbuf = NULL;

  G_OBJECT_CLASS (tweet_status_info_parent_class)->constructed (gobject);

  font_name = NULL;
  tidy_stylable_get (TIDY_STYLABLE (gobject), "font-name", &font_name, NULL);
  if (!font_name)
    font_name = g_strdup ("Sans 8");

  g_assert (TWITTER_IS_STATUS (info->status));

  user = twitter_status_get_user (info->status);
  g_assert (TWITTER_IS_USER (user));

  /* icon */
  pixbuf = twitter_user_get_profile_image (user);
  if (pixbuf)
    info->icon = tweet_texture_new_from_pixbuf (pixbuf);
  else
    {
      info->icon = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (info->icon),
                                   &text_color);
    }

  clutter_container_add_actor (CLUTTER_CONTAINER (gobject), info->icon);
  clutter_actor_set_size (info->icon, ICON_WIDTH, ICON_HEIGHT);
  clutter_actor_set_position (info->icon,
                              (MAX_WIDTH - ICON_WIDTH) / 2,
                              ICON_Y);

  /* some twitter client does not escape bare '&' properly, so we
   * get failures from the pango markup parser. we need to replace
   * the '&\s' with a corresponding '&amp; '.
   */
  escaped = g_regex_replace (info->escape_re,
                             twitter_status_get_text (info->status), -1,
                             0,
                             "&amp;",
                             0,
                             NULL);

  twitter_date_to_time_val (twitter_status_get_created_at (info->status), &timeval);

  created_at = tweet_format_time_for_display (&timeval);
  text = g_strdup_printf ("<b>%s (%s)</b> wrote:\n"
                          "%s\n"
                          "\n"
                          "<small>%s</small>",
                          twitter_user_get_name (user),
                          twitter_user_get_screen_name (user),
                          escaped,
                          created_at);
  g_free (created_at);
  g_free (escaped);

  info->label = clutter_label_new ();
  clutter_label_set_color (CLUTTER_LABEL (info->label), &text_color);
  clutter_label_set_font_name (CLUTTER_LABEL (info->label), font_name);
  clutter_label_set_line_wrap (CLUTTER_LABEL (info->label), TRUE);
  clutter_label_set_line_wrap_mode (CLUTTER_LABEL (info->label), PANGO_WRAP_WORD_CHAR);  
  clutter_label_set_text (CLUTTER_LABEL (info->label), text);
  clutter_label_set_use_markup (CLUTTER_LABEL (info->label), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (gobject), info->label);
  clutter_actor_set_position (info->label, TEXT_X, TEXT_Y);
  clutter_actor_set_size (info->label, TEXT_WIDTH, 1);

  g_free (text);

  info->star_button =
    tweet_texture_new_from_icon_name (NULL, "favorite-status", -1);
  if (!info->star_button)
    {
      info->star_button = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (info->star_button),
                                   &text_color);
    }

  g_object_set_data_full (G_OBJECT (info->star_button), "button-tip",
                          g_strdup ("Favorite this update"),
                          g_free);
  g_signal_connect (info->star_button,
                    "enter-event", G_CALLBACK (on_button_enter),
                    info);
  g_signal_connect (info->star_button,
                    "leave-event", G_CALLBACK (on_button_leave),
                    info);
  g_signal_connect (info->star_button,
                    "button-press-event", G_CALLBACK (on_button_press),
                    info);
  clutter_container_add_actor (CLUTTER_CONTAINER (gobject), info->star_button);
  clutter_actor_set_size (info->star_button, BUTTON_SIZE, BUTTON_SIZE);
  clutter_actor_set_position (info->star_button,
                              TEXT_X,
                              0);
  clutter_actor_set_reactive (info->star_button, TRUE);
  clutter_actor_show (info->star_button);

  info->reply_button =
    tweet_texture_new_from_icon_name (NULL, "reply-to-status", -1);
  if (!info->reply_button)
    {
      info->reply_button = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (info->reply_button),
                                   &text_color);
    }

  g_object_set_data_full (G_OBJECT (info->reply_button), "button-tip",
                          g_strdup_printf ("Reply to %s", twitter_user_get_name (user)),
                          g_free);
  g_signal_connect (info->reply_button,
                    "enter-event", G_CALLBACK (on_button_enter),
                    info);
  g_signal_connect (info->reply_button,
                    "leave-event", G_CALLBACK (on_button_leave),
                    info);
  g_signal_connect (info->reply_button,
                    "button-press-event", G_CALLBACK (on_button_press),
                    info);
  clutter_container_add_actor (CLUTTER_CONTAINER (gobject), info->reply_button);
  clutter_actor_set_size (info->reply_button, BUTTON_SIZE, BUTTON_SIZE);
  clutter_actor_set_position (info->reply_button,
                              TEXT_WIDTH - BUTTON_SIZE,
                              0);
  clutter_actor_set_reactive (info->reply_button, TRUE);
  clutter_actor_show (info->reply_button);

  info->button_tip = clutter_label_new ();
  clutter_label_set_color (CLUTTER_LABEL (info->button_tip), &text_color);
  clutter_label_set_font_name (CLUTTER_LABEL (info->button_tip), font_name);
  clutter_label_set_line_wrap (CLUTTER_LABEL (info->button_tip), TRUE);
  clutter_label_set_text (CLUTTER_LABEL (info->button_tip), "");
  clutter_label_set_use_markup (CLUTTER_LABEL (info->button_tip), TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (info), info->button_tip);
  clutter_actor_set_position (info->button_tip, TEXT_WIDTH / 2, 0);
  clutter_actor_set_size (info->button_tip, TEXT_WIDTH - (2 * BUTTON_SIZE), 1);
  clutter_actor_set_opacity (info->button_tip, 0);
  clutter_actor_show (info->button_tip);

  g_free (font_name);
}

static void
tweet_status_info_request_coords (ClutterActor    *actor,
                                  ClutterActorBox *box)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (actor);
  ClutterUnit width, height;

  CLUTTER_ACTOR_CLASS (tweet_status_info_parent_class)->request_coords (actor, box);

  width = CLUTTER_UNITS_TO_DEVICE (box->x2 - box->x1);
  height = CLUTTER_UNITS_TO_DEVICE (box->y2 - box->y1);

  clutter_actor_set_position (info->icon,
                              (width - ICON_WIDTH - (2 * H_PADDING)) / 2,
                              ICON_Y);

  clutter_actor_set_width (info->label, (width - (2 * H_PADDING)));
  clutter_actor_set_height (info->label, -1);

  if (height >= (ICON_Y + ICON_HEIGHT))
    clutter_actor_show (info->icon);
  else
    clutter_actor_hide (info->icon);

  if (height >= TEXT_Y)
    clutter_actor_show (info->label);
  else
    clutter_actor_hide (info->label);

  clutter_actor_set_position (info->reply_button,
                              TEXT_X + H_PADDING,
                              height - BUTTON_SIZE - V_PADDING);
  clutter_actor_set_position (info->star_button,
                              width - BUTTON_SIZE - H_PADDING,
                              height - BUTTON_SIZE - V_PADDING);
  clutter_actor_set_y (info->button_tip, height - BUTTON_SIZE - V_PADDING);

  info->allocation = *box;
}

static void
tweet_status_info_class_init (TweetStatusInfoClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->constructed = tweet_status_info_constructed;
  gobject_class->set_property = tweet_status_info_set_property;
  gobject_class->get_property = tweet_status_info_get_property;
  gobject_class->dispose = tweet_status_info_dispose;

  actor_class->request_coords = tweet_status_info_request_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_STATUS,
                                   g_param_spec_object ("status",
                                                        "Status",
                                                        "Twitter status",
                                                        TWITTER_TYPE_STATUS,
                                                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  info_signals[STAR_CLICKED] =
    g_signal_new ("star-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TweetStatusInfoClass, star_clicked),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  info_signals[REPLY_CLICKED] =
    g_signal_new ("reply-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TweetStatusInfoClass, reply_clicked),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
tweet_status_info_init (TweetStatusInfo *info)
{
  info->escape_re = g_regex_new ("&(?!(amp|gt|lt|apos))", 0, 0, NULL);
}

ClutterActor *
tweet_status_info_new (TwitterStatus *status)
{
  g_return_val_if_fail (TWITTER_IS_STATUS (status), NULL);

  return g_object_new (TWEET_TYPE_STATUS_INFO, "status", status, NULL);
}

void
tweet_status_info_set_status (TweetStatusInfo *info,
                              TwitterStatus   *status)
{
  g_return_if_fail (TWEET_IS_STATUS_INFO (info));
  g_return_if_fail (TWITTER_IS_STATUS (status));

  if (info->status)
    g_object_unref (info->status);

  info->status = g_object_ref (status);

  if (CLUTTER_ACTOR_IS_VISIBLE (info))
    clutter_actor_queue_redraw (CLUTTER_ACTOR (info));

  g_object_notify (G_OBJECT (info), "status");
}

TwitterStatus *
tweet_status_info_get_status (TweetStatusInfo *info)
{
  g_return_val_if_fail (TWEET_IS_STATUS_INFO (info), NULL);

  return info->status;
}

