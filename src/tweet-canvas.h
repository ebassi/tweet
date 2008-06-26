#ifndef __TWEET_CANVAS_H__
#define __TWEET_CANVAS_H__

#include <gtk/gtk.h>
#include <clutter-gtk/gtk-clutter-embed.h>

G_BEGIN_DECLS

#define TWEET_TYPE_CANVAS               (tweet_canvas_get_type ())
#define TWEET_CANVAS(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TWEET_TYPE_CANVAS, TweetCanvas))
#define TWEET_IS_CANVAS(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TWEET_TYPE_CANVAS))
#define TWEET_CANVAS_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), TWEET_TYPE_CANVAS, TweetCanvasClass))
#define TWEET_IS_CANVAS_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), TWEET_TYPE_CANVAS))
#define TWEET_CANVAS_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), TWEET_TYPE_CANVAS, TweetCanvasClass))

#define TWEET_CANVAS_MIN_WIDTH          350
#define TWEET_CANVAS_MIN_HEIGHT         500

typedef struct _TweetCanvas             TweetCanvas;
typedef struct _TweetCanvasClass        TweetCanvasClass;

struct _TweetCanvas
{
  GtkClutterEmbed parent_instance;

  guint border_width;
};

struct _TweetCanvasClass
{
  GtkClutterEmbedClass parent_class;
};

GType      tweet_canvas_get_type         (void) G_GNUC_CONST;
GtkWidget *tweet_canvas_new              (void);
void       tweet_canvas_set_border_width (TweetCanvas *canvas,
                                          guint        border_width);
guint      tweet_canvas_get_border_width (TweetCanvas *canvas);

G_END_DECLS

#endif /* __TWEET_CANVAS_H__ */
