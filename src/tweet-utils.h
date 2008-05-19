#ifndef __TWEET_UTILS_H__
#define __TWEET_UTILS_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkstyle.h>
#include <clutter/clutter-actor.h>
#include <clutter/clutter-color.h>

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

ClutterActor *tweet_texture_new_from_pixbuf    (GdkPixbuf   *pixbuf);
ClutterActor *tweet_texture_new_from_stock     (GtkWidget   *widget,
                                                const gchar *stock_id,
                                                GtkIconSize  size);
ClutterActor *tweet_texture_new_from_icon_name (GtkWidget   *widget,
                                                const gchar *icon_name,
                                                GtkIconSize  size);

G_END_DECLS

#endif /* __TWEET_UTILS_H__ */
