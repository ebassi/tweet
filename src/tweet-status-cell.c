/* tweet-status-cell.c: Cell actor for the status renderer
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
#include <tidy/tidy-stylable.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-status-cell.h"
#include "tweet-utils.h"

#define ICON_WIDTH      48
#define ICON_HEIGHT     48

#define V_PADDING       6
#define H_PADDING       12

#define DEFAULT_WIDTH   (96 + (2 * H_PADDING) + 230)
#define DEFAULT_HEIGHT  (72 + (2 * V_PADDING))

#define ICON_X          (H_PADDING / 2)
#define ICON_Y          (V_PADDING + 12)

#define TEXT_X          (ICON_WIDTH + (2 * H_PADDING))
#define TEXT_Y          (V_PADDING)

#define BG_ROUND_RADIUS 12

enum
{
  PROP_0,

  PROP_STATUS,
  PROP_FONT_NAME
};

G_DEFINE_TYPE (TweetStatusCell, tweet_status_cell, CLUTTER_TYPE_GROUP);

static void
tweet_status_cell_dispose (GObject *gobject)
{
  TweetStatusCell *cell = TWEET_STATUS_CELL (gobject);

  g_free (cell->font_name);

  if (cell->escape_re)
    {
      g_regex_unref (cell->escape_re);
      cell->escape_re = NULL;
    }

  if (cell->status)
    {
      g_object_unref (cell->status);
      cell->status = NULL;
    }

  G_OBJECT_CLASS (tweet_status_cell_parent_class)->dispose (gobject);
}

static void
tweet_status_cell_set_property (GObject      *gobject,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  TweetStatusCell *cell = TWEET_STATUS_CELL (gobject);

  switch (prop_id)
    {
    case PROP_STATUS:
      cell->status = g_value_dup_object (value);
      break;

    case PROP_FONT_NAME:
      cell->font_name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_status_cell_get_property (GObject    *gobject,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  TweetStatusCell *cell = TWEET_STATUS_CELL (gobject);

  switch (prop_id)
    {
    case PROP_STATUS:
      g_value_set_object (value, cell->status);
      break;

    case PROP_FONT_NAME:
      g_value_set_string (value, cell->font_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
tweet_status_cell_constructed (GObject *gobject)
{
  TweetStatusCell *cell = TWEET_STATUS_CELL (gobject);
  cairo_t *cr;
  cairo_pattern_t *pat;
  ClutterColor bg_color = { 162, 162, 162, 0xcc };
  ClutterColor text_color = { 0, 0, 0, 255 };
  TwitterUser *user;
  gchar *text, *created_at, *escaped;
  GTimeVal timeval = { 0, };
  GdkPixbuf *pixbuf = NULL;
  gint width = DEFAULT_WIDTH;
  gint height = DEFAULT_HEIGHT;

  g_assert (TWITTER_IS_STATUS (cell->status));

  user = twitter_status_get_user (cell->status);
  g_assert (TWITTER_IS_USER (user));

  /* some twitter client doesn't escape bare '&' properly, so we get
   * failures from the pango markup parser. we need to replace the
   * '&\s' with corresponding '&amp; '.
   */
  escaped = g_regex_replace (cell->escape_re,
                             twitter_status_get_text (cell->status), -1,
                             0,
                             "&amp;",
                             0,
                             NULL);

  twitter_date_to_time_val (twitter_status_get_created_at (cell->status), &timeval);

  created_at = tweet_format_time_for_display (&timeval);
  text = g_strdup_printf ("<b>%s</b> %s <small>%s</small>",
                          twitter_user_get_screen_name (user),
                          escaped,
                          created_at);
  g_free (created_at);
  g_free (escaped);

  cell->label = clutter_label_new ();
  clutter_label_set_color (CLUTTER_LABEL (cell->label), &text_color);
  clutter_label_set_font_name (CLUTTER_LABEL (cell->label), cell->font_name);
  clutter_label_set_line_wrap (CLUTTER_LABEL (cell->label), TRUE);
  clutter_label_set_text (CLUTTER_LABEL (cell->label), text);
  clutter_label_set_use_markup (CLUTTER_LABEL (cell->label), TRUE);
  clutter_actor_set_position (cell->label, TEXT_X, TEXT_Y);
  clutter_actor_set_size (cell->label, 230, 1);
  clutter_actor_show (cell->label);

  height =
    MAX (DEFAULT_HEIGHT, clutter_actor_get_height (cell->label) + 2 * V_PADDING);

  g_free (text);

  /* icon */
  pixbuf = twitter_user_get_profile_image (user);
  if (pixbuf)
    cell->icon = tweet_texture_new_from_pixbuf (pixbuf);
  else
    {
      cell->icon = clutter_rectangle_new ();
      clutter_rectangle_set_color (CLUTTER_RECTANGLE (cell->icon), &text_color);
    }

  clutter_actor_set_size (cell->icon, ICON_WIDTH, ICON_HEIGHT);
  clutter_actor_set_position (cell->icon,
                              ICON_X,
                              (height - ICON_HEIGHT) / 2);
  clutter_actor_show (cell->icon);
  clutter_actor_set_reactive (cell->icon, TRUE);

  cell->cell_height = CLUTTER_UNITS_FROM_DEVICE (height);

  /* background texture */
  cell->bg = clutter_cairo_new (width, height);
  clutter_actor_show (cell->bg);

  cr = clutter_cairo_create (CLUTTER_CAIRO (cell->bg));
  g_assert (cr != NULL);

  width = DEFAULT_WIDTH - (H_PADDING / 2);

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

  /* bubble texture */
  cell->bubble = clutter_cairo_new (width, height);
  clutter_actor_set_position (cell->bubble, TEXT_X - H_PADDING, 0);
  clutter_actor_show (cell->bubble);

  cr = clutter_cairo_create (CLUTTER_CAIRO (cell->bubble));
  g_assert (cr != NULL);

  pat = cairo_pattern_create_linear (0, 0, 0, height);
  cairo_pattern_add_color_stop_rgba (pat, 1, .85, .85, .85, 1);
  cairo_pattern_add_color_stop_rgba (pat, .85, 1, 1, 1, 1);
  cairo_pattern_add_color_stop_rgba (pat, .15, 1, 1, 1, 1);
  cairo_pattern_add_color_stop_rgba (pat, 0, .85, .85, .85, 1);

  {
    gint x = H_PADDING / 2;
    gint y = 0;

    width = DEFAULT_WIDTH - (ICON_WIDTH + (2 * H_PADDING));

    /*
    context.move_to(x+r,y)                      # Move to A
    context.line_to(x+w-r,y)                    # Straight line to B
    context.curve_to(x+w,y,x+w,y,x+w,y+r)       # Curve to C, Control points are both at Q
    context.line_to(x+w,y+h-r)                  # Move to D
    context.curve_to(x+w,y+h,x+w,y+h,x+w-r,y+h) # Curve to E
    context.line_to(x+r,y+h)                    # Line to F
    context.curve_to(x,y+h,x,y+h,x,y+h-r)       # Curve to G
    context.line_to(x,y+r)                      # Line to H
    context.curve_to(x,y,x,y,x+r,y)             # Curve to A
    */
    cairo_move_to (cr, x + BG_ROUND_RADIUS, y);
    cairo_line_to (cr, x + width - BG_ROUND_RADIUS, y);
    cairo_curve_to (cr, x + width, y, x + width, y, x + width, y + BG_ROUND_RADIUS);
    cairo_line_to (cr, x + width, y + height - BG_ROUND_RADIUS);
    cairo_curve_to (cr, x + width, y + height, x + width, y + height, x + width - BG_ROUND_RADIUS, y + height);
    cairo_line_to (cr, x + BG_ROUND_RADIUS, y + height);
    cairo_curve_to (cr, x, y + height, x, y + height, x, y + height - BG_ROUND_RADIUS);

      cairo_line_to (cr, x, y + (((height - BG_ROUND_RADIUS * 2.0) / 3.0) * 2.0));
      cairo_line_to (cr, 0, y + ((height - BG_ROUND_RADIUS * 2.0) / 2.0));
      cairo_line_to (cr, x, y + (((height - BG_ROUND_RADIUS * 2.0) / 3.0)));

    cairo_line_to (cr, x, y + BG_ROUND_RADIUS);
    cairo_curve_to (cr, x, y, x, y, x + BG_ROUND_RADIUS, y);
  }

  cairo_close_path (cr);

  cairo_set_source (cr, pat);
  cairo_fill_preserve (cr);

  cairo_pattern_destroy (pat);
  cairo_destroy (cr);

  /* we add them in the right order */
  clutter_container_add (CLUTTER_CONTAINER (cell),
                         cell->bg,
                         cell->icon,
                         cell->bubble,
                         cell->label,
                         NULL);
}

static void
tweet_status_cell_query_coords (ClutterActor    *actor,
                                ClutterActorBox *box)
{
  TweetStatusCell *cell = TWEET_STATUS_CELL (actor);

  box->x2 = box->x1 + CLUTTER_UNITS_FROM_DEVICE (DEFAULT_WIDTH);
  box->y2 = box->y1 + cell->cell_height;
}

static void
tweet_status_cell_class_init (TweetStatusCellClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  gobject_class->constructed = tweet_status_cell_constructed;
  gobject_class->set_property = tweet_status_cell_set_property;
  gobject_class->get_property = tweet_status_cell_get_property;
  gobject_class->dispose = tweet_status_cell_dispose;

  actor_class->query_coords = tweet_status_cell_query_coords;

  g_object_class_install_property (gobject_class,
                                   PROP_FONT_NAME,
                                   g_param_spec_string ("font-name",
                                                        "Font Name",
                                                        "Font name",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_STATUS,
                                   g_param_spec_object ("status",
                                                        "Status",
                                                        "Twitter status",
                                                        TWITTER_TYPE_STATUS,
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
}

static void
tweet_status_cell_init (TweetStatusCell *cell)
{
  cell->escape_re = g_regex_new ("&(?!(amp|gt|lt|apos))", 0, 0, NULL);
}

ClutterActor *
tweet_status_cell_new (TwitterStatus *status,
                       const gchar   *font_name)
{
  g_return_val_if_fail (TWITTER_IS_STATUS (status), NULL);

  return g_object_new (TWEET_TYPE_STATUS_CELL,
                       "status", status,
                       "font-name", font_name,
                       NULL);
}
