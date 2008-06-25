/* tweet-utils.c: Utility API
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

#include <glib.h>
#include <glib/gi18n.h>

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
  color->alpha = 255;
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

void
tweet_texture_set_from_pixbuf (ClutterTexture *texture,
                               GdkPixbuf      *pixbuf)
{
  GError *error;

  g_return_if_fail (CLUTTER_IS_TEXTURE (texture));
  g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

  error = NULL;
  clutter_texture_set_from_rgb_data (texture,
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

void
tweet_texture_set_from_stock (ClutterTexture *texture,
                              GtkWidget      *widget,
                              const gchar    *stock_id,
                              GtkIconSize     size)
{
  GdkPixbuf *pixbuf;

  g_return_if_fail (CLUTTER_IS_TEXTURE (texture));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (stock_id != NULL);
  g_return_if_fail (size > GTK_ICON_SIZE_INVALID || size == -1);

  pixbuf = gtk_widget_render_icon (widget, stock_id, size, NULL);
  if (!pixbuf)
    pixbuf = gtk_widget_render_icon (widget,
                                     GTK_STOCK_MISSING_IMAGE, size,
                                     NULL);

  tweet_texture_set_from_pixbuf (texture, pixbuf);
  g_object_unref (pixbuf);
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

  g_return_val_if_fail (widget == NULL || GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (icon_name != NULL, NULL);
  g_return_val_if_fail (size > GTK_ICON_SIZE_INVALID || size == -1, NULL);

  if (widget && gtk_widget_has_screen (widget))
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

  if (size == -1 ||
      !gtk_icon_size_lookup_for_settings (settings, size, &width, &height))
    {
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

      if (widget)
        return tweet_texture_new_from_stock (widget,
                                             GTK_STOCK_MISSING_IMAGE,
                                             size);
      else
        return NULL;
    }

  retval = tweet_texture_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  return retval; 
}

void
tweet_texture_set_from_icon_name (ClutterTexture *texture,
                                  GtkWidget      *widget,
                                  const gchar    *icon_name,
                                  GtkIconSize     size)
{
  GtkSettings *settings;
  GtkIconTheme *icon_theme;
  gint width, height;
  GdkPixbuf *pixbuf;
  GError *error;

  g_return_if_fail (CLUTTER_IS_TEXTURE (texture));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));
  g_return_if_fail (icon_name != NULL);
  g_return_if_fail (size > GTK_ICON_SIZE_INVALID || size == -1);

  if (widget && gtk_widget_has_screen (widget))
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

  if (size == -1 ||
      !gtk_icon_size_lookup_for_settings (settings, size, &width, &height))
    {
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

      if (widget)
        return tweet_texture_set_from_stock (texture,
                                             widget,
                                             GTK_STOCK_MISSING_IMAGE,
                                             size);
      else
        return;
    }

  tweet_texture_set_from_pixbuf (texture, pixbuf);
  g_object_unref (pixbuf);
}

void
tweet_show_url (GtkWidget      *widget,
                const gchar    *link_)
{
  GdkScreen *screen;
  gint pid;
  GError *error;
  gchar **argv;

  if (widget && gtk_widget_has_screen (widget))
    screen = gtk_widget_get_screen (widget);
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

gchar *
tweet_format_time_for_display (GTimeVal *time_)
{
  GTimeVal now;
  struct tm tm_mtime;
  const gchar *format = NULL;
  gchar *locale_format = NULL;
  gchar buf[256];
  gchar *retval = NULL;
  gint secs_diff;

  g_return_val_if_fail (time_->tv_usec >= 0 && time_->tv_usec < G_USEC_PER_SEC, NULL);

  g_get_current_time (&now);

#ifdef HAVE_LOCALTIME_R
  localtime_r ((time_t *) &(time_->tv_sec), &tm_mtime);
#else
  {
    struct tm *ptm = localtime ((time_t *) &(time_->tv_usec));

    if (!ptm)
      {
        g_warning ("ptm != NULL failed");
        return NULL;
      }
    else
      memcpy ((void *) &tm_mtime, (void *) ptm, sizeof (struct tm));
  }
#endif /* HAVE_LOCALTIME_R */

  secs_diff  = now.tv_sec - time_->tv_sec;

  /* within the hour */
  if (secs_diff < 60)
    retval = g_strdup (_("Less than a minute ago"));
  else
    {
      gint mins_diff = secs_diff / 60;

      if (mins_diff < 60)
        retval = g_strdup_printf (ngettext ("About a minute ago",
                                            "About %d minutes ago",
                                            mins_diff),
                                  mins_diff);
      else if (mins_diff < 360)
        {
          gint hours_diff = mins_diff / 60;

          retval = g_strdup_printf (ngettext ("About an hour ago",
                                              "About %d hours ago",
                                              hours_diff),
                                    hours_diff);
        }
    }

  if (retval)
    return retval;
  else
    {
      GDate d1, d2;
      gint days_diff;

      g_date_set_time_t (&d1, now.tv_sec);
      g_date_set_time_t (&d2, time_->tv_sec);

      days_diff = g_date_get_julian (&d1) - g_date_get_julian (&d2);

      if (days_diff == 0)
        format = _("Today at %H:%M");
      else if (days_diff == 1)
        format = _("Yesterday at %H:%M");
      else
        {
          if (days_diff > 1 && days_diff < 7)
            format = _("Last %A at %H:%M"); /* day of the week */
          else
            format = _("%x at %H:%M"); /* any other date */
        }
    }

  locale_format = g_locale_from_utf8 (format, -1, NULL, NULL, NULL);

  if (strftime (buf, sizeof (buf), locale_format, &tm_mtime) != 0)
    retval = g_locale_to_utf8 (buf, -1, NULL, NULL, NULL);
  else
    retval = g_strdup (_("Unknown"));

  g_free (locale_format);

  return retval;
}
