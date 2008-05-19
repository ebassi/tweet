#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <gtk/gtk.h>
#include <clutter/clutter.h>

#include "tweet-utils.h"

static inline void
tweet_widget_get_component (GtkWidget    *widget,
                            GtkRcFlags    component,
                            GtkStateType  state,
                            ClutterColor *color)
{
  GtkStyle *style = gtk_widget_get_style (widget);
  GdkColor gtk_color = { 0, };

  switch (component)
    {
    case GTK_RC_FG:
      gtk_color = style->fg[state];
      break;

    case GTK_RC_BG:
      gtk_color = style->bg[state];
      break;

    case GTK_RC_TEXT:
      gtk_color = style->text[state];
      break;

    case GTK_RC_BASE:
      gtk_color = style->base[state];
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  color->red   = CLAMP (((gtk_color.red   / 65535.0) * 255), 0, 255);
  color->green = CLAMP (((gtk_color.green / 65535.0) * 255), 0, 255);
  color->blue  = CLAMP (((gtk_color.blue  / 65535.0) * 255), 0, 255);
}

void
tweet_widget_get_fg_color (GtkWidget    *widget,
                           GtkStateType  state,
                           ClutterColor *color)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (state >= GTK_STATE_NORMAL &&
                    state <= GTK_STATE_INSENSITIVE);
  g_return_if_fail (color != NULL);

  tweet_widget_get_component (widget, GTK_RC_FG, state, color);
}

void
tweet_widget_get_bg_color (GtkWidget    *widget,
                           GtkStateType  state,
                           ClutterColor *color)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (state >= GTK_STATE_NORMAL &&
                    state <= GTK_STATE_INSENSITIVE);
  g_return_if_fail (color != NULL);

  tweet_widget_get_component (widget, GTK_RC_BG, state, color);
}

void
tweet_widget_get_text_color (GtkWidget    *widget,
                             GtkStateType  state,
                             ClutterColor *color)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (state >= GTK_STATE_NORMAL &&
                    state <= GTK_STATE_INSENSITIVE);
  g_return_if_fail (color != NULL);

  tweet_widget_get_component (widget, GTK_RC_TEXT, state, color);
}

void
tweet_widget_get_base_color (GtkWidget    *widget,
                             GtkStateType  state,
                             ClutterColor *color)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (state >= GTK_STATE_NORMAL &&
                    state <= GTK_STATE_INSENSITIVE);
  g_return_if_fail (color != NULL);

  tweet_widget_get_component (widget, GTK_RC_BASE, state, color);
}

ClutterActor *
tweet_texture_new_from_pixbuf (GdkPixbuf *pixbuf)
{
  ClutterActor *retval;
  GError *error;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

  retval = clutter_texture_new ();

  error = NULL;
  clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (retval),
                                     gdk_pixbuf_get_pixels (pixbuf),
                                     gdk_pixbuf_get_has_alpha (pixbuf),
                                     gdk_pixbuf_get_width (pixbuf),
                                     gdk_pixbuf_get_height (pixbuf),
                                     gdk_pixbuf_get_rowstride (pixbuf),
                                     4, 0,
                                     &error);
  if (error)
    {
      g_warning ("Unable to set the pixbuf: %s", error->message);
      g_error_free (error);
    }

  return retval; 
}

ClutterActor *
tweet_texture_new_from_stock (GtkWidget   *widget,
                              const gchar *stock_id,
                              GtkIconSize  size)
{
  GdkPixbuf *pixbuf;
  ClutterActor *retval;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);
  g_return_val_if_fail (size > GTK_ICON_SIZE_INVALID || size == -1, NULL);

  pixbuf = gtk_widget_render_icon (widget, stock_id, size, NULL);
  if (!pixbuf)
    pixbuf = gtk_widget_render_icon (widget,
                                     GTK_STOCK_MISSING_IMAGE, size,
                                     NULL);

  retval = tweet_texture_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  return retval;
}

ClutterActor *
tweet_texture_new_from_icon_name (GtkWidget   *widget,
                                  const gchar *icon_name,
                                  GtkIconSize  size)
{
  GtkSettings *settings;
  GtkIconTheme *icon_theme;
  gint width, height;
  GdkPixbuf *pixbuf;
  GError *error;
  ClutterActor *retval;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (icon_name != NULL, NULL);
  g_return_val_if_fail (size > GTK_ICON_SIZE_INVALID || size == -1, NULL);

  if (gtk_widget_has_screen (widget))
    {
      GdkScreen *screen;

      screen = gtk_widget_get_screen (widget);
      settings = gtk_settings_get_for_screen (screen);
      icon_theme = gtk_icon_theme_get_for_screen (screen);
    }
  else
    {
      settings = gtk_settings_get_default ();
      icon_theme = gtk_icon_theme_get_default ();
    }

  if (!gtk_icon_size_lookup_for_settings (settings, size, &width, &height))
    {
      g_warning ("Invalid icon size");
      width = height = 48;
    }

  error = NULL;
  pixbuf = gtk_icon_theme_load_icon (icon_theme,
                                     icon_name,
                                     MIN (width, height), 0,
                                     &error);
  if (error)
    {
      g_error_free (error);
      return tweet_texture_new_from_stock (widget,
                                           GTK_STOCK_MISSING_IMAGE,
                                           size);
    }

  retval = tweet_texture_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  return retval; 
}
