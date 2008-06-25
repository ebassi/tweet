/* tweet-url-label.h: Subclass of ClutterLabel with clickable URLs
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
#include <string.h>

#include "tweet-url-label.h"

static void tweet_url_label_dispose (GObject *object);
static void tweet_url_label_finalize (GObject *object);
static void tweet_url_label_notify (GObject *object, GParamSpec *pspec);
static void tweet_url_label_paint (ClutterActor *actor);

typedef struct _TweetUrlLabelMatch TweetUrlLabelMatch;

struct _TweetUrlLabelMatch
{
  gint start, end;
};

/* Taken from http://tinyurl.com/2ss83t */
static const char tweet_url_label_regex[] =
  "\\b\n"
  "# Match the leading part (proto://hostname, or just hostname)\n"
  "(\n"
  "  # http://, or https:// leading part\n"
  "  (https?)://[-\\w]+(\\.\\w[-\\w]*)+\n"
  "|\n"
  "  # or, try to find a hostname with more specific sub-expression\n"
  "  (?i: [a-z0-9] (?:[-a-z0-9]*[a-z0-9])? \\. )+ # sub domains\n"
  "  # Now ending .com, etc. For these, require lowercase\n"
  "  (?-i: com\\b\n"
  "      | edu\\b\n"
  "      | biz\\b\n"
  "      | gov\\b\n"
  "      | in(?:t|fo)\\b # .int or .info\n"
  "      | mil\\b\n"
  "      | net\\b\n"
  "      | org\\b\n"
  "      | [a-z][a-z]\\.[a-z][a-z]\\b # two-letter country code\n"
  "  )\n"
  ")\n"

  "# Allow an optional port number\n"
  "( : \\d+ )?\n"

  "# The rest of the URL is optional, and begins with /\n"
  "(\n"
  "  /\n"
  "  # The rest are heuristics for what seems to work well\n"
  "  [^.!,?;\"'<>()\\[\\]{}\\s\\x7F-\\xFF]*\n"
  "  (\n"
  "    [.!,?]+ [^.!,?;\"\\'<>()\\[\\]{}\\s\\x7F-\\xFF]+\n"
  "  )*\n"
  ")?\n";

G_DEFINE_TYPE (TweetUrlLabel, tweet_url_label, CLUTTER_TYPE_LABEL);

static void
tweet_url_label_class_init (TweetUrlLabelClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  ClutterActorClass *actor_class = (ClutterActorClass *) klass;

  gobject_class->dispose = tweet_url_label_dispose;
  gobject_class->finalize = tweet_url_label_finalize;
  gobject_class->notify = tweet_url_label_notify;

  actor_class->paint = tweet_url_label_paint;
}

static void
tweet_url_label_init (TweetUrlLabel *self)
{
  GError *error = NULL;

  self->url_regex = g_regex_new (tweet_url_label_regex,
				 G_REGEX_CASELESS
				 | G_REGEX_EXTENDED
				 | G_REGEX_NO_AUTO_CAPTURE,
				 0, &error);

  if (self->url_regex == NULL)
    {
      g_critical ("Compilation of URL matching regex failed: %s",
		  error->message);
      g_error_free (error);
      error = NULL;
    }

  self->matches = g_array_new (FALSE, FALSE, sizeof (TweetUrlLabelMatch));
}

ClutterActor *
tweet_url_label_new (void)
{
  return g_object_new (TWEET_TYPE_URL_LABEL, NULL);
}

static void
tweet_url_label_paint (ClutterActor *actor)
{
  TweetUrlLabel *self = TWEET_URL_LABEL (actor);
  PangoLayout *layout;
  int i;

  /* Set the attributes in the label's layout so that the URLs will be
     in blue */
  if (self->matches->len > 0)
    {
      PangoAttrList *attrs;

      layout = clutter_label_get_layout (CLUTTER_LABEL (self));

      attrs = pango_layout_get_attributes (layout);
      if (attrs == NULL)
	attrs = pango_attr_list_new ();
      else
	pango_attr_list_ref (attrs);

      for (i = 0; i < self->matches->len; i++)
	{
	  TweetUrlLabelMatch *match = &g_array_index (self->matches,
						      TweetUrlLabelMatch, i);
	  PangoAttribute *attr = pango_attr_foreground_new (0, 0, 65535);

	  attr->start_index = match->start;
	  attr->end_index = match->end;
	  pango_attr_list_change (attrs, attr);
	}

      pango_layout_set_attributes (layout, attrs);

      pango_attr_list_unref (attrs);
    }

  CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)->paint (actor);
}

static void
tweet_url_label_update_matches (TweetUrlLabel *self)
{
  /* Clear any existing matches */
  g_array_set_size (self->matches, 0);

  if (self->url_regex)
    {
      GMatchInfo *match_info;
      PangoLayout *layout;
      const gchar *text;

      /* Get the text of the label from the layout so that it won't
	 include the markup */
      layout = clutter_label_get_layout (CLUTTER_LABEL (self));
      text = pango_layout_get_text (layout);

      /* Find each URL and keep track of its location */
      g_regex_match (self->url_regex, text, 0, &match_info);
      while (g_match_info_matches (match_info))
	{
	  TweetUrlLabelMatch match;

	  if (g_match_info_fetch_pos (match_info, 0, &match.start, &match.end))
	    g_array_append_val (self->matches, match);

	  g_match_info_next (match_info, NULL);
	}
      g_match_info_free (match_info);
    }
}

static void
tweet_url_label_notify (GObject *object, GParamSpec *pspec)
{
  /* Recalculate the positions of the URLs if the text changes */
  if (!strcmp (pspec->name, "text") || !strcmp (pspec->name, "use-markup"))
    tweet_url_label_update_matches (TWEET_URL_LABEL (object));

  if (G_OBJECT_CLASS (tweet_url_label_parent_class)->notify)
    G_OBJECT_CLASS (tweet_url_label_parent_class)->notify (object, pspec);
}

static void
tweet_url_label_dispose (GObject *object)
{
  TweetUrlLabel *self = (TweetUrlLabel *) object;

  if (self->url_regex)
    {
      g_object_unref (self->url_regex);
      self->url_regex = NULL;
    }

  G_OBJECT_CLASS (tweet_url_label_parent_class)->dispose (object);
}

static void
tweet_url_label_finalize (GObject *object)
{
  TweetUrlLabel *self = (TweetUrlLabel *) object;

  g_array_free (self->matches, TRUE);

  G_OBJECT_CLASS (tweet_url_label_parent_class)->finalize (object);
}
