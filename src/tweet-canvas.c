#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>

#include "tweet-canvas.h"

enum
{
  PROP_0,

  PROP_BORDER_WIDTH
};

G_DEFINE_TYPE (TweetCanvas, tweet_canvas, GTK_TYPE_CLUTTER_EMBED);

static void
tweet_canvas_size_request (GtkWidget      *widget,
                           GtkRequisition *request)
{
  TweetCanvas *canvas = TWEET_CANVAS (widget);

  request->width = TWEET_CANVAS_MIN_WIDTH
                 + (2 * canvas->border_width);

  request->height = TWEET_CANVAS_MIN_HEIGHT
                  + (2 * canvas->border_width);
}

static void
tweet_canvas_set_property (GObject      *gobject,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  TweetCanvas *canvas = TWEET_CANVAS (gobject);

  switch (prop_id)
    {
    case PROP_BORDER_WIDTH:
      tweet_canvas_set_border_width (canvas, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
tweet_canvas_get_property (GObject    *gobject,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  TweetCanvas *canvas = TWEET_CANVAS (gobject);

  switch (prop_id)
    {
    case PROP_BORDER_WIDTH:
      g_value_set_uint (value, canvas->border_width);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
tweet_canvas_class_init (TweetCanvasClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = tweet_canvas_set_property;
  gobject_class->get_property = tweet_canvas_get_property;

  widget_class->size_request = tweet_canvas_size_request;

  g_object_class_install_property (gobject_class,
                                   PROP_BORDER_WIDTH,
                                   g_param_spec_uint ("border-width",
                                                      "Border Width",
                                                      "Width of the border",
                                                      0, G_MAXUINT,
                                                      0,
                                                      G_PARAM_READWRITE));
}

static void
tweet_canvas_init (TweetCanvas *canvas)
{
  canvas->border_width = 0;
}

GtkWidget *
tweet_canvas_new (void)
{
  return g_object_new (TWEET_TYPE_CANVAS, NULL);
}

void
tweet_canvas_set_border_width (TweetCanvas *canvas,
                               guint        border_width)
{
  g_return_if_fail (TWEET_IS_CANVAS (canvas));

  if (canvas->border_width != border_width)
    {
      canvas->border_width = border_width;
      g_object_notify (G_OBJECT (canvas), "border-width");

      if (GTK_WIDGET_REALIZED (canvas))
        gtk_widget_queue_resize (GTK_WIDGET (canvas));
    }
}

guint
tweet_canvas_get_border_width (TweetCanvas *canvas)
{
  g_return_val_if_fail (TWEET_IS_CANVAS (canvas), 0);

  return canvas->border_width;
}
