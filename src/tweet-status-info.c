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

#define MAX_WIDTH       350

#define ICON_WIDTH      64
#define ICON_HEIGHT     64

#define V_PADDING       6
#define H_PADDING       12

#define ICON_Y          (V_PADDING)

#define TEXT_X          (H_PADDING)
#define TEXT_Y          (ICON_Y + ICON_HEIGHT + (2 * V_PADDING))
#define TEXT_WIDTH      (MAX_WIDTH * (2 * H_PADDING))

#define BG_ROUND_RADIUS 24

enum
{
  PROP_0,

  PROP_STATUS
};

G_DEFINE_TYPE (TweetStatusInfo, tweet_status_info, TIDY_TYPE_ACTOR);

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

  if (info->bg)
    {
      clutter_actor_destroy (info->bg);
      clutter_actor_destroy (info->icon);
      clutter_actor_destroy (info->label);

      info->bg = info->icon = info->label = NULL;
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

static void
draw_background (ClutterCairo *background)
{
  ClutterColor bg_color = { 255, 255, 255, 196 };
  cairo_t *cr;
  gint width, height;

  cr = clutter_cairo_create (background);
  g_assert (cr != NULL);

  width = height = 0;
  g_object_get (G_OBJECT (background),
                "surface-width", &width,
                "surface-height", &height,
                NULL);

  cairo_move_to (cr, BG_ROUND_RADIUS, 0);
  cairo_line_to (cr, width - BG_ROUND_RADIUS, 0);
  cairo_curve_to (cr, width, 0, width, 0, width, BG_ROUND_RADIUS);
  cairo_line_to (cr, width, height - BG_ROUND_RADIUS);
  cairo_curve_to (cr, width, height, width, height, width - BG_ROUND_RADIUS, height);
  cairo_line_to (cr, BG_ROUND_RADIUS, height);
  cairo_curve_to (cr, 0, height, 0, height, 0, height - BG_ROUND_RADIUS);
  cairo_line_to (cr, 0, BG_ROUND_RADIUS);
  cairo_curve_to (cr, 0, 0, 0, 0, BG_ROUND_RADIUS, 0);

  cairo_close_path (cr);

  clutter_cairo_set_source_color (cr, &bg_color);
  cairo_fill_preserve (cr);

  cairo_destroy (cr);
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

  font_name = NULL;
  tidy_stylable_get (TIDY_STYLABLE (gobject), "font-name", &font_name, NULL);
  if (!font_name)
    font_name = g_strdup ("Sans 8");

  /* background texture */
  info->bg = clutter_cairo_new (MAX_WIDTH, 10);
  clutter_actor_set_parent (info->bg, CLUTTER_ACTOR (info));
  clutter_actor_set_size (info->bg, MAX_WIDTH, 10);
  clutter_actor_set_position (info->bg, 0, 0);
  clutter_actor_show (info->bg);

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
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (info->icon), &text_color);
    }

  clutter_actor_set_parent (info->icon, CLUTTER_ACTOR (info));
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
                             "&amp; ",
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
  clutter_label_set_text (CLUTTER_LABEL (info->label), text);
  clutter_label_set_use_markup (CLUTTER_LABEL (info->label), TRUE);
  clutter_actor_set_parent (info->label, CLUTTER_ACTOR (info));
  clutter_actor_set_position (info->label, TEXT_X, TEXT_Y);
  clutter_actor_set_width (info->label, TEXT_WIDTH);
  clutter_actor_show (info->label);

  g_free (font_name);
  g_free (text);
}

static void
tweet_status_info_query_coords (ClutterActor    *actor,
                                ClutterActorBox *box)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (actor);

  box->x2 = box->x1 + (info->allocation.x2 - info->allocation.x1);
  box->y2 = box->y1 + (info->allocation.y2 - info->allocation.y1);
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

  clutter_cairo_surface_resize (CLUTTER_CAIRO (info->bg), width, height);
  clutter_actor_set_size (info->bg, width, height);
  draw_background (CLUTTER_CAIRO (info->bg));

  if (height > (ICON_Y + ICON_HEIGHT))
    clutter_actor_show (info->icon);
  else
    clutter_actor_hide (info->icon);

  if (height > (TEXT_Y + clutter_actor_get_height (info->label)))
    {
      clutter_actor_set_width (info->label, (width - (2 * H_PADDING)));
      clutter_actor_show (info->label);
    }
  else
    clutter_actor_hide (info->label);

#if 0
  if (height > (BUTTONS_Y + BUTTONS_HEIGHT))
    {
      clutter_actor_show (info->reply_button);
      clutter_actor_show (info->star_button);
    }
  else
    {
      clutter_actor_hide (info->reply_button);
      clutter_actor_hide (info->star_button);
    }
#endif

  info->allocation = *box;
}

static void
tweet_status_info_paint (ClutterActor *actor)
{
  TweetStatusInfo *info = TWEET_STATUS_INFO (actor);

  if (CLUTTER_ACTOR_IS_VISIBLE (info->bg))
    clutter_actor_paint (info->bg);

  if (CLUTTER_ACTOR_IS_VISIBLE (info->icon))
    clutter_actor_paint (info->icon);

  if (CLUTTER_ACTOR_IS_VISIBLE (info->label))
    clutter_actor_paint (info->label);
}

static void
tweet_status_info_pick (ClutterActor *actor,
                        const ClutterColor *pick_color)
{
  if (!clutter_actor_should_pick_paint (actor))
    return;

  CLUTTER_ACTOR_CLASS (tweet_status_info_parent_class)->pick (actor, pick_color);
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

  actor_class->query_coords = tweet_status_info_query_coords;
  actor_class->request_coords = tweet_status_info_request_coords;
  actor_class->pick = tweet_status_info_pick;
  actor_class->paint = tweet_status_info_paint;

  g_object_class_install_property (gobject_class,
                                   PROP_STATUS,
                                   g_param_spec_object ("status",
                                                        "Status",
                                                        "Twitter status",
                                                        TWITTER_TYPE_STATUS,
                                                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}

static void
tweet_status_info_init (TweetStatusInfo *info)
{
  info->escape_re = g_regex_new ("&\\s", 0, 0, NULL);
}

ClutterActor *
tweet_status_info_new (TwitterStatus *status)
{
  g_return_val_if_fail (TWITTER_IS_STATUS (status), NULL);

  return g_object_new (TWEET_TYPE_STATUS_INFO, "status", status, NULL);
}
