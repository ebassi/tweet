/* tweet-status-model.h: Model for the status view
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

#include <glib-object.h>

#include <clutter/clutter.h>

#include "tweet-status-model.h"

#define TWEET_TYPE_STATUS_MODEL_ITER                 \
        (tweet_status_model_iter_get_type())
#define TWEET_STATUS_MODEL_ITER(obj)                 \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),           \
         TWEET_TYPE_STATUS_MODEL_ITER,               \
         TweetStatusModelIter))
#define TWEET_IS_STATUS_MODEL_ITER(obj)              \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),           \
         TWEET_TYPE_STATUS_MODEL_ITER))
#define TWEET_STATUS_MODEL_ITER_CLASS(klass)         \
        (G_TYPE_CHECK_CLASS_CAST ((klass),           \
         TWEET_TYPE_STATUS_MODEL_ITER,               \
         TweetStatusModelIterClass))
#define TWEET_IS_STATUS_MODEL_ITER_CLASS(klass)      \
        (G_TYPE_CHECK_CLASS_TYPE ((klass),           \
         TWEET_TYPE_STATUS_MODEL_ITER))
#define TWEET_STATUS_MODEL_ITER_GET_CLASS(obj)       \
        (G_TYPE_INSTANCE_GET_CLASS ((obj),           \
         TWEET_TYPE_STATUS_MODEL_ITER,               \
         TweetStatusModelIterClass))

typedef struct _TweetStatusModelIter    TweetStatusModelIter;
typedef struct _ClutterModelIterClass   TweetStatusModelIterClass;

struct _TweetStatusModelIter
{
  ClutterModelIter parent_instance;

  GSequenceIter *seq_iter;
};

#define TWEET_STATUS_MODEL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TWEET_TYPE_STATUS_MODEL, TweetStatusModelPrivate))

struct _TweetStatusModelPrivate
{
  GSequence *sequence;

  gint max_size;
};

static const gchar *model_names[] = {
  "Status"
};

static const gint model_columns = G_N_ELEMENTS (model_names);



/*
 * TweetStatusModel
 */

G_DEFINE_TYPE (TweetStatusModelIter,
               tweet_status_model_iter,
               CLUTTER_TYPE_MODEL_ITER);

static void
tweet_status_model_iter_get_value (ClutterModelIter *iter,
                                   guint             column,
                                   GValue           *value)
{
  TweetStatusModelIter *iter_default;
  GValueArray *value_array;
  GValue *iter_value;
  GValue real_value = { 0, };
  gboolean converted = FALSE;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  value_array = g_sequence_get (iter_default->seq_iter);
  iter_value = g_value_array_get_nth (value_array, column);
  g_assert (iter_value != NULL);

  if (!g_type_is_a (G_VALUE_TYPE (value), G_VALUE_TYPE (iter_value)))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), 
                                    G_VALUE_TYPE (iter_value)) &&
          !g_value_type_compatible (G_VALUE_TYPE (iter_value), 
                                    G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          return;
        }

      if (!g_value_transform (iter_value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s",
                     G_STRLOC, 
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          g_value_unset (&real_value);
        }

      converted = TRUE;
    }
  
  if (converted)
    {
      g_value_copy (&real_value, value);
      g_value_unset (&real_value);
    }
  else
    g_value_copy (iter_value, value);
}

static void
tweet_status_model_iter_set_value (ClutterModelIter *iter,
                                   guint             column,
                                   const GValue     *value)
{
  TweetStatusModelIter *iter_default;
  GValueArray *value_array;
  GValue *iter_value;
  GValue real_value = { 0, };
  gboolean converted = FALSE;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  value_array = g_sequence_get (iter_default->seq_iter);
  iter_value = g_value_array_get_nth (value_array, column);
  g_assert (iter_value != NULL);

  if (!g_type_is_a (G_VALUE_TYPE (value), G_VALUE_TYPE (iter_value)))
    {
      if (!g_value_type_compatible (G_VALUE_TYPE (value), 
                                    G_VALUE_TYPE (iter_value)) &&
          !g_value_type_compatible (G_VALUE_TYPE (iter_value), 
                                    G_VALUE_TYPE (value)))
        {
          g_warning ("%s: Unable to convert from %s to %s\n",
                     G_STRLOC,
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          return;
        }

      if (!g_value_transform (value, &real_value))
        {
          g_warning ("%s: Unable to make conversion from %s to %s\n",
                     G_STRLOC, 
                     g_type_name (G_VALUE_TYPE (value)),
                     g_type_name (G_VALUE_TYPE (iter_value)));
          g_value_unset (&real_value);
        }

      converted = TRUE;
    }
 
  if (converted)
    {
      g_value_copy (&real_value, iter_value);
      g_value_unset (&real_value);
    }
  else
    g_value_copy (value, iter_value);
}

static gboolean
tweet_status_model_iter_is_first (ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  ClutterModel *model;
  ClutterModelIter *temp_iter;
  GSequenceIter *begin, *end;
  guint row;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter);

  begin = g_sequence_get_begin_iter (TWEET_STATUS_MODEL (model)->priv->sequence);
  end   = iter_default->seq_iter;

  temp_iter = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (begin))
    {
      TWEET_STATUS_MODEL_ITER (temp_iter)->seq_iter = begin;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        {
          end = begin;
          break;
        }

      begin = g_sequence_iter_next (begin);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* This is because the 'begin_iter' is always *before* the last valid
   * iter, otherwise we'd have endless loops 
   */
  end = g_sequence_iter_prev (end);

  return iter_default->seq_iter == end;
}

static gboolean
tweet_status_model_iter_is_last (ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model;
  GSequenceIter *begin, *end;
  guint row;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  if (g_sequence_iter_is_end (iter_default->seq_iter))
    return TRUE;

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter);

  begin = g_sequence_get_end_iter (TWEET_STATUS_MODEL (model)->priv->sequence);
  begin = g_sequence_iter_prev (begin);
  end   = iter_default->seq_iter;

  temp_iter = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (begin))
    {
      TWEET_STATUS_MODEL_ITER (temp_iter)->seq_iter = begin;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        {
          end = begin;
          break;
        }

      begin = g_sequence_iter_prev (begin);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* This is because the 'end_iter' is always *after* the last valid iter.
   * Otherwise we'd have endless loops 
   */
  end = g_sequence_iter_next (end);

  return iter_default->seq_iter == end;
}

static ClutterModelIter *
tweet_status_model_iter_next (ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model = NULL;
  GSequenceIter *filter_next;
  guint row;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter) + 1;

  filter_next = g_sequence_iter_next (iter_default->seq_iter);
  g_assert (filter_next != NULL);

  temp_iter = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_end (filter_next))
    {
      TWEET_STATUS_MODEL_ITER (temp_iter)->seq_iter = filter_next;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        break;

      filter_next = g_sequence_iter_next (filter_next);
      row += 1;
    }

  g_object_unref (temp_iter);

  /* We do this because the 'end_iter' is always *after* the
   * last valid iter. Otherwise loops will go on forever
   */
  if (filter_next == iter_default->seq_iter)
    filter_next = g_sequence_iter_next (filter_next);

  /* update the iterator and return it */
  g_object_set (G_OBJECT (iter_default), "model", model, "row", row, NULL);
  iter_default->seq_iter = filter_next;

  return CLUTTER_MODEL_ITER (iter_default);
}

static ClutterModelIter *
tweet_status_model_iter_prev (ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  ClutterModelIter *temp_iter;
  ClutterModel *model;
  GSequenceIter *filter_prev;
  guint row;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);
  g_assert (iter_default->seq_iter != NULL);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter) - 1;

  filter_prev = g_sequence_iter_prev (iter_default->seq_iter);
  g_assert (filter_prev != NULL);

  temp_iter = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                            "model", model,
                            NULL);

  while (!g_sequence_iter_is_begin (filter_prev))
    {
      TWEET_STATUS_MODEL_ITER (temp_iter)->seq_iter = filter_prev;
      g_object_set (G_OBJECT (temp_iter), "row", row, NULL);

      if (clutter_model_filter_iter (model, temp_iter))
        break;

      filter_prev = g_sequence_iter_prev (filter_prev);
      row -= 1;
    }

  g_object_unref (temp_iter);

  /* We do this because the 'end_iter' is always *after* the last
   * valid iter. Otherwise loops will go on forever
   */
  if (filter_prev == iter_default->seq_iter)
    filter_prev = g_sequence_iter_prev (filter_prev);

  /* update the iterator and return it */
  g_object_set (G_OBJECT (iter_default), "model", model, "row", row, NULL);
  iter_default->seq_iter = filter_prev;

  return CLUTTER_MODEL_ITER (iter_default);
}

#if CLUTTER_CHECK_VERSION(0, 7, 0)
static ClutterModelIter *
tweet_status_model_iter_copy (ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  TweetStatusModelIter *iter_copy;
  ClutterModel *model;
  guint row;
 
  iter_default = TWEET_STATUS_MODEL_ITER (iter);

  model = clutter_model_iter_get_model (iter);
  row   = clutter_model_iter_get_row (iter) - 1;

  iter_copy = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                            "model", model,
                            "row", row,
                            NULL);

  /* this is safe, because the seq_iter pointer on the passed
   * iterator will be always be overwritten in ::next or ::prev
   */
  iter_copy->seq_iter = iter_default->seq_iter;

  return CLUTTER_MODEL_ITER (iter_copy);
}
#endif /* CLUTTER_CHECK_VERSION(0, 7, 0) */

static void
tweet_status_model_iter_class_init (TweetStatusModelIterClass *klass)
{
  ClutterModelIterClass *iter_class = CLUTTER_MODEL_ITER_CLASS (klass);

  iter_class->get_value = tweet_status_model_iter_get_value;
  iter_class->set_value = tweet_status_model_iter_set_value;
  iter_class->is_first  = tweet_status_model_iter_is_first;
  iter_class->is_last   = tweet_status_model_iter_is_last;
  iter_class->next      = tweet_status_model_iter_next;
  iter_class->prev      = tweet_status_model_iter_prev;

#if CLUTTER_CHECK_VERSION(0, 7, 0)
  iter_class->copy      = tweet_status_model_iter_copy;
#endif
}

static void
tweet_status_model_iter_init (TweetStatusModelIter *iter)
{
  iter->seq_iter = NULL;
}

/*
 * TweetStatusModel
 */

G_DEFINE_TYPE (TweetStatusModel, tweet_status_model, CLUTTER_TYPE_MODEL);

static ClutterModelIter *
tweet_status_model_get_iter_at_row (ClutterModel *model,
                                    guint         row)
{
  TweetStatusModelPrivate *priv = TWEET_STATUS_MODEL (model)->priv;
  TweetStatusModelIter *retval;

  if (row >= g_sequence_get_length (priv->sequence))
    return NULL;

  retval = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                         "model", model,
                         "row", row,
                         NULL);
  retval->seq_iter = g_sequence_get_iter_at_pos (priv->sequence, row);

  return CLUTTER_MODEL_ITER (retval);
}

static ClutterModelIter *
tweet_status_model_insert_row (ClutterModel *model,
                               gint          index_)
{
  TweetStatusModelPrivate *priv = TWEET_STATUS_MODEL (model)->priv;
  TweetStatusModelIter *retval;
  guint n_columns, i, pos;
  GValueArray *array;
  GSequenceIter *seq_iter;

  n_columns = clutter_model_get_n_columns (model);
  array = g_value_array_new (n_columns);

  for (i = 0; i < n_columns; i++)
    {
      GValue *value = NULL;

      g_value_array_append (array, NULL);

      value = g_value_array_get_nth (array, i);
      g_value_init (value, clutter_model_get_column_type (model, i));
    }

  if (index_ < 0)
    {
      seq_iter = g_sequence_append (priv->sequence, array);
      pos = g_sequence_get_length (priv->sequence);
    }
  else if (index_ == 0)
    {
      seq_iter = g_sequence_prepend (priv->sequence, array);
      pos = 0;
    }
  else
    {
      seq_iter = g_sequence_get_iter_at_pos (priv->sequence, index_);
      seq_iter = g_sequence_insert_before (seq_iter, array);
      pos = index_;
    }

  retval = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                         "model", model,
                         "row", pos,
                         NULL);
  retval->seq_iter = seq_iter;

  return CLUTTER_MODEL_ITER (retval);
}

static void
tweet_status_model_remove_row (ClutterModel *model,
                               guint         row)
{
  TweetStatusModelPrivate *priv = TWEET_STATUS_MODEL (model)->priv;
  GSequenceIter *seq_iter;
  guint pos = 0;

  seq_iter = g_sequence_get_begin_iter (priv->sequence);
  while (!g_sequence_iter_is_end (seq_iter))
    {
      if (clutter_model_filter_row (model, pos))
        {
          if (pos == row)
            {  
              ClutterModelIter *iter;

              iter = g_object_new (TWEET_TYPE_STATUS_MODEL_ITER,
                                   "model", model,
                                   "row", pos,
                                   NULL);
              TWEET_STATUS_MODEL_ITER (iter)->seq_iter = seq_iter;

              /* the actual row is removed from the sequence inside
               * the ::row-removed signal class handler, so that every
               * handler connected to ::row-removed will still get
               * a valid iterator, and every signal connected to
               * ::row-removed with the AFTER flag will get an updated
               * model
               */
              g_signal_emit_by_name (model, "row-removed", iter);

              g_object_unref (iter);

              break;
            }
        }

      pos += 1;
      seq_iter = g_sequence_iter_next (seq_iter);
    }
}

static guint
tweet_status_model_get_n_rows (ClutterModel *model)
{
  TweetStatusModelPrivate *priv = TWEET_STATUS_MODEL (model)->priv;

  return g_sequence_get_length (priv->sequence);
}

typedef struct
{
  ClutterModel *model;
  guint column;
  ClutterModelSortFunc func;
  gpointer data;
} SortClosure;

static gint
sort_model_default (gconstpointer a,
                    gconstpointer b,
                    gpointer      data)
{
  GValueArray *row_a = (GValueArray *) a;
  GValueArray *row_b = (GValueArray *) b;
  SortClosure *clos = data;

  return clos->func (clos->model,
                     g_value_array_get_nth (row_a, clos->column),
                     g_value_array_get_nth (row_b, clos->column),
                     clos->data);
}

static void
tweet_status_model_resort (ClutterModel         *model,
                           ClutterModelSortFunc  func,
                           gpointer              data)
{
  SortClosure sort_closure = { NULL, 0, NULL, NULL };

  sort_closure.model  = model;
  sort_closure.column = clutter_model_get_sorting_column (model);
  sort_closure.func   = func;
  sort_closure.data   = data;

  g_sequence_sort (TWEET_STATUS_MODEL (model)->priv->sequence,
                   sort_model_default,
                   &sort_closure);
}

static void
tweet_status_model_row_removed (ClutterModel     *model,
                                ClutterModelIter *iter)
{
  TweetStatusModelIter *iter_default;
  GValueArray *array;

  iter_default = TWEET_STATUS_MODEL_ITER (iter);

  array = g_sequence_get (iter_default->seq_iter);
  g_value_array_free (array);

  g_sequence_remove (iter_default->seq_iter);
  iter_default->seq_iter = NULL;
}

static void
tweet_status_model_finalize (GObject *gobject)
{
  TweetStatusModelPrivate *priv = TWEET_STATUS_MODEL (gobject)->priv;
  GSequenceIter *iter;

  iter = g_sequence_get_begin_iter (priv->sequence);
  while (!g_sequence_iter_is_end (iter))
    {
      GValueArray *value_array = g_sequence_get (iter);

      g_value_array_free (value_array);
      iter = g_sequence_iter_next (iter);
    }
  g_sequence_free (priv->sequence);

  G_OBJECT_CLASS (tweet_status_model_parent_class)->finalize (gobject);
}

static void
tweet_status_model_class_init (TweetStatusModelClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterModelClass *model_class = CLUTTER_MODEL_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TweetStatusModelPrivate));

  gobject_class->finalize = tweet_status_model_finalize;

  model_class->get_n_rows      = tweet_status_model_get_n_rows;
  model_class->get_iter_at_row = tweet_status_model_get_iter_at_row;
  model_class->insert_row      = tweet_status_model_insert_row;
  model_class->remove_row      = tweet_status_model_remove_row;
  model_class->resort          = tweet_status_model_resort;

  model_class->row_removed     = tweet_status_model_row_removed;
}

static void
tweet_status_model_init (TweetStatusModel *model)
{
  ClutterModel *base_model = CLUTTER_MODEL (model);
  TweetStatusModelPrivate *priv;
  GType model_types[] = {
    TWITTER_TYPE_STATUS
  };

  model->priv = priv = TWEET_STATUS_MODEL_GET_PRIVATE (model);

  priv->sequence = g_sequence_new (NULL);

  clutter_model_set_types (base_model, model_columns, model_types);
  clutter_model_set_names (base_model, model_columns, model_names);
}

ClutterModel *
tweet_status_model_new (void)
{
  return g_object_new (TWEET_TYPE_STATUS_MODEL, NULL);
}

static void
status_changed_cb (TwitterStatus    *status,
                   TweetStatusModel *model)
{
  ClutterModelIter *iter;

  iter = clutter_model_get_first_iter (CLUTTER_MODEL (model));
  while (!clutter_model_iter_is_last (iter))
    {
      TwitterStatus *iter_status;
      guint iter_status_id, status_id;

      clutter_model_iter_get (iter, 0, &iter_status, -1);
      if (!iter_status)
        {
          clutter_model_iter_next (iter);
          continue;
        }

      iter_status_id = twitter_status_get_id (iter_status);
      status_id = twitter_status_get_id (status);

      if (iter_status == status || iter_status_id == status_id)
        g_signal_emit_by_name (model, "row-changed", iter);

      g_object_unref (iter_status);

      clutter_model_iter_next (iter);
    }

  g_object_unref (iter);
}

static gboolean
tweet_status_model_lookup_status (TweetStatusModel *model,
                                  TwitterStatus    *status)
{
  ClutterModelIter *iter;
  gboolean retval = FALSE;

  iter = clutter_model_get_first_iter (CLUTTER_MODEL (model));
  if (!iter)
    return FALSE;

  while (!clutter_model_iter_is_last (iter))
    {
      TwitterStatus *s;

      clutter_model_iter_get (iter, 0, &s, -1);
      if (twitter_status_get_id (s) == twitter_status_get_id (status))
        {
          retval = TRUE;
          g_object_unref (s);
          break;
        }

      g_object_unref (s);
      clutter_model_iter_next (iter);
    }

  g_object_unref (iter);

  return retval;
}

void
tweet_status_model_append_status (TweetStatusModel *model,
                                  TwitterStatus    *status)
{
  g_return_if_fail (TWEET_IS_STATUS_MODEL (model));
  g_return_if_fail (TWITTER_IS_STATUS (status));

  if (tweet_status_model_lookup_status (model, status))
    return;

  clutter_model_append (CLUTTER_MODEL (model), 0, status, -1);
  g_signal_connect (status, "changed",
                    G_CALLBACK (status_changed_cb),
                    model);
}

void
tweet_status_model_prepend_status (TweetStatusModel *model,
                                   TwitterStatus    *status)
{
  g_return_if_fail (TWEET_IS_STATUS_MODEL (model));
  g_return_if_fail (TWITTER_IS_STATUS (status));

  if (tweet_status_model_lookup_status (model, status))
    return;

  clutter_model_prepend (CLUTTER_MODEL (model), 0, status, -1);
  g_signal_connect (status, "changed",
                    G_CALLBACK (status_changed_cb),
                    model);
}

TwitterStatus *
tweet_status_model_get_status (TweetStatusModel *model,
                               ClutterModelIter *iter)
{
  TwitterStatus *status;

  g_return_val_if_fail (TWEET_IS_STATUS_MODEL (model), NULL);
  g_return_val_if_fail (TWEET_IS_STATUS_MODEL_ITER (iter), NULL);

  status = NULL;
  clutter_model_iter_get (iter, 0, &status, -1);

  return status;
}

void
tweet_status_model_set_max_size (TweetStatusModel *model,
                                 gint              max_size)
{
  TweetStatusModelPrivate *priv;

  g_return_if_fail (TWEET_IS_STATUS_MODEL (model));

  priv = model->priv;
  if (priv->max_size != max_size)
    {
      priv->max_size = max_size;
    }
}
