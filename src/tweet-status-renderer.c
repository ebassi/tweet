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
#include <tidy/tidy-cell-renderer.h>
#include <tidy/tidy-frame.h>
#include <tidy/tidy-list-view.h>
#include <tidy/tidy-stylable.h>

#include <twitter-glib/twitter-glib.h>

#include "tweet-status-cell.h"
#include "tweet-status-renderer.h"
#include "tweet-utils.h"

G_DEFINE_TYPE (TweetStatusRenderer,
               tweet_status_renderer,
               TIDY_TYPE_CELL_RENDERER);

static ClutterActor *
tweet_status_renderer_get_cell_actor (TidyCellRenderer *renderer,
                                      TidyActor        *list_view,
                                      const GValue     *value,
                                      TidyCellState     cell_state,
                                      ClutterGeometry  *size,
                                      gint              row,
                                      gint              column)
{
  ClutterActor *retval = NULL;
  gchar *font_name;
  ClutterColor *background_color, *active_color, *text_color;
  ClutterFixed x_align, y_align;

  tidy_stylable_get (TIDY_STYLABLE (list_view),
                     "font-name", &font_name,
                     "bg-color", &background_color,
                     "active-color", &active_color,
                     "text-color", &text_color,
                     NULL);

  tidy_cell_renderer_get_alignmentx (renderer, &x_align, &y_align);

  if (row == -1 || cell_state == TIDY_CELL_HEADER)
    {
      ClutterActor *label;

      retval = tidy_frame_new ();
      tidy_actor_set_alignmentx (TIDY_ACTOR (retval), x_align, y_align);

      label = g_object_new (CLUTTER_TYPE_LABEL,
                            "font-name", font_name,
                            "text", g_value_get_string (value),
                            "color", text_color,
                            "alignment", PANGO_ALIGN_CENTER,
                            "ellipsize", PANGO_ELLIPSIZE_END,
                            "wrap", FALSE,
                            NULL);
      clutter_container_add_actor (CLUTTER_CONTAINER (retval), label);
      clutter_actor_show (label);

      goto out;
    }

  if (G_VALUE_TYPE (value) != TWITTER_TYPE_STATUS)
    return NULL;
  else
    retval = tweet_status_cell_new (g_value_get_object (value), font_name);

out:
  g_free (font_name);
  clutter_color_free (background_color);
  clutter_color_free (active_color);
  clutter_color_free (text_color);

  return retval;
}

static void
tweet_status_renderer_class_init (TweetStatusRendererClass *klass)
{
  TidyCellRendererClass *renderer_class = TIDY_CELL_RENDERER_CLASS (klass);

  renderer_class->get_cell_actor = tweet_status_renderer_get_cell_actor;
}

static void
tweet_status_renderer_init (TweetStatusRenderer *renderer)
{
}

TidyCellRenderer *
tweet_status_renderer_new (void)
{
  return g_object_new (TWEET_TYPE_STATUS_RENDERER, NULL);
}
