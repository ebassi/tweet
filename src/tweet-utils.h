/* tweet-utils.h: Utility API
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
#ifndef __TWEET_UTILS_H__
#define __TWEET_UTILS_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkstyle.h>
#include <clutter/clutter-actor.h>
#include <clutter/clutter-color.h>
#include <clutter/clutter-texture.h>

G_BEGIN_DECLS

void tweet_widget_get_fg_color   (GtkWidget    *widget,
                                  GtkStateType  state,
                                  ClutterColor *color);
void tweet_widget_get_bg_color   (GtkWidget    *widget,
                                  GtkStateType  state,
                                  ClutterColor *color);
void tweet_widget_get_text_color (GtkWidget    *widget,
                                  GtkStateType  state,
                                  ClutterColor *color);
void tweet_widget_get_base_color (GtkWidget    *widget,
                                  GtkStateType  state,
                                  ClutterColor *color);

ClutterActor *tweet_texture_new_from_pixbuf    (GdkPixbuf      *pixbuf);
ClutterActor *tweet_texture_new_from_stock     (GtkWidget      *widget,
                                                const gchar    *stock_id,
                                                GtkIconSize     size);
ClutterActor *tweet_texture_new_from_icon_name (GtkWidget      *widget,
                                                const gchar    *icon_name,
                                                GtkIconSize     size);
void          tweet_texture_set_from_pixbuf    (ClutterTexture *texture,
                                                GdkPixbuf      *pixbuf);
void          tweet_texture_set_from_stock     (ClutterTexture *texture,
                                                GtkWidget      *widget,
                                                const gchar    *stock_id,
                                                GtkIconSize     size);
void          tweet_texture_set_from_icon_name (ClutterTexture *texture,
                                                GtkWidget      *widget,
                                                const gchar    *icon_name,
                                                GtkIconSize     size);

gchar *tweet_format_time_for_display (GTimeVal *time_);

void tweet_show_url (GtkWidget      *widget,
		     const gchar    *link_);

G_END_DECLS

#endif /* __TWEET_UTILS_H__ */
