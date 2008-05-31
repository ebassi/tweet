#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "tweet-config.h"
#include "tweet-preferences.h"

#define I_(str) (g_intern_static_string ((str)))

static void
on_refresh_combo_changed (GtkComboBox *combo_box,
                          TweetConfig *config)
{
  GtkTreeIter iter;
  gint seconds;

  if (!gtk_combo_box_get_active_iter (combo_box, &iter))
    return;

  /* column 0 is the text, column 1 is the duration in seconds */
  gtk_tree_model_get (gtk_combo_box_get_model (combo_box), &iter,
                      1, &seconds,
                      -1);

  tweet_config_set_refresh_time (config, seconds);
}

static void
on_close (GtkDialog *dialog,
          gint       response_id,
          gpointer   user_data)
{
  gtk_widget_hide (GTK_WIDGET (dialog));
}

void
tweet_show_preferences_dialog (GtkWindow   *parent,
                               const gchar *title,
                               TweetConfig *config)
{
  GtkWidget *dialog;

  dialog = g_object_get_data (G_OBJECT (parent), I_("tweet-preferences-dialog"));

  if (!dialog)
    {
      GtkBuilder *builder;
      GError *error;
      GtkWidget *combo;
      guint res;

      builder = gtk_builder_new ();

      error =  NULL;
      res = gtk_builder_add_from_file (builder,
                                       PKGDATADIR G_DIR_SEPARATOR_S "tweet-preferences.xml",
                                       &error);
      if (error)
        {
          g_warning ("Unable to create the preferences dialog: %s",
                     error->message);
          g_error_free (error);
          g_object_unref (builder);
          return;
        }

      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "preferences-dialog"));
      g_object_ref_sink (dialog);
      g_signal_connect (dialog, "response", G_CALLBACK (on_close), NULL);

      gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
      gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
      gtk_window_set_title (GTK_WINDOW (dialog), title);
      g_object_set_data_full (G_OBJECT (parent),
                              I_("tweet-preferences-dialog"),
                              dialog, g_object_unref);

      combo = GTK_WIDGET (gtk_builder_get_object (builder, "refresh-combo"));
      g_signal_connect (combo,
                        "changed", G_CALLBACK (on_refresh_combo_changed),
                        config);

      g_object_unref (builder);
    }

  gtk_window_present (GTK_WINDOW (dialog));
}
