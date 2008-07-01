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

#include <tidy/tidy-list-view.h>
#include <tidy/tidy-adjustment.h>
#include <gdk/gdkcursor.h>
#include <gdk/gdkdisplay.h>

#include "tweet-url-label.h"
#include "tweet-utils.h"
#include "tweet-hot-actor.h"

static void tweet_hot_actor_iface_init (TweetHotActorIface *iface);
static void tweet_url_label_dispose (GObject *object);
static void tweet_url_label_finalize (GObject *object);
static void tweet_url_label_notify (GObject *object, GParamSpec *pspec);
static void tweet_url_label_paint (ClutterActor *actor);
static gboolean tweet_url_label_motion_event (ClutterActor *Actor,
					      ClutterMotionEvent *event);
static gboolean tweet_url_label_leave_event (ClutterActor *actor,
					     ClutterCrossingEvent *event);
static gboolean tweet_url_label_button_press_event (ClutterActor *actor,
						    ClutterButtonEvent *event);
static GdkCursor *tweet_url_label_get_cursor (TweetHotActor *actor,
					      GdkDisplay    *display,
					      int            x,
					      int            y);

typedef struct _TweetUrlLabelMatch TweetUrlLabelMatch;

struct _TweetUrlLabelMatch
{
  gint start, end;
};

/* Taken from http://tinyurl.com/2ss83t */
static const char tweet_url_label_regex[] =
  "\\b\n"
  /* Match the leading part (proto://hostname, or just hostname) */
  "(\n"
  /* http://, or https:// leading part */
  "  (https?)://[-\\w]+(\\.\\w[-\\w]*)+\n"
  "|\n"
  /* or, try to find a hostname with more specific sub-expression */
  "  (?i: [a-z0-9] (?:[-a-z0-9]*[a-z0-9])? \\. )+\n" /* sub domains */
  /* Now ending .com, etc. For these, require lowercase */
  "  (?-i: com\\b\n"
  "      | edu\\b\n"
  "      | biz\\b\n"
  "      | gov\\b\n"
  "      | in(?:t|fo)\\b\n" /* .int or .info */
  "      | mil\\b\n"
  "      | net\\b\n"
  "      | org\\b\n"
  "      | [a-z][a-z]\\.[a-z][a-z]\\b\n" /* two-letter country code */
  "  )\n"
  ")\n"

  /* Allow an optional port number */
  "( : \\d+ )?\n"

  /* The rest of the URL is optional, and begins with */
  "(\n"
  "  /\n"
  /* The rest are heuristics for what seems to work well */
  "  [^.!,?;\"'<>()\\[\\]{}\\s\\x7F-\\xFF]*\n"
  "  (\n"
  "    [.!,?]+ [^.!,?;\"\\'<>()\\[\\]{}\\s\\x7F-\\xFF]+\n"
  "  )*\n"
  ")?\n";

G_DEFINE_TYPE_WITH_CODE (TweetUrlLabel,
			 tweet_url_label,
			 CLUTTER_TYPE_LABEL,
			 G_IMPLEMENT_INTERFACE (TWEET_TYPE_HOT_ACTOR,
						tweet_hot_actor_iface_init));

static void
tweet_url_label_class_init (TweetUrlLabelClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  ClutterActorClass *actor_class = (ClutterActorClass *) klass;

  gobject_class->dispose = tweet_url_label_dispose;
  gobject_class->finalize = tweet_url_label_finalize;
  gobject_class->notify = tweet_url_label_notify;

  actor_class->paint = tweet_url_label_paint;
  actor_class->motion_event = tweet_url_label_motion_event;
  actor_class->leave_event = tweet_url_label_leave_event;
  actor_class->button_press_event = tweet_url_label_button_press_event;
}

static void
tweet_hot_actor_iface_init (TweetHotActorIface *iface)
{
  iface->get_cursor = tweet_url_label_get_cursor;
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
  self->selected_match = -1;
  self->hand_cursor = NULL;
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

	  /* If the cursor is over this URL then also make it
	     underlined */
	  if (i == self->selected_match)
	    {
	      attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
	      attr->start_index = match->start;
	      attr->end_index = match->end;
	      pango_attr_list_change (attrs, attr);
	    }
	}

      pango_layout_set_attributes (layout, attrs);

      pango_attr_list_unref (attrs);
    }

  CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)->paint (actor);
}

static gboolean
tweet_url_label_motion_event (ClutterActor *actor, ClutterMotionEvent *event)
{
  TweetUrlLabel *self = TWEET_URL_LABEL (actor);
  gint new_selected_match = -1;
  ClutterActor *parent_actor;

  if (self->matches->len > 0)
    {
      gint actor_x, actor_y, layout_x, layout_y;
      gint index, i;
      PangoLayout *layout;

      /* Simplistically convert the cursor position from
	 stage-relative to actor-relative. This won't work if the
	 actor is scaled or rotated */
      clutter_actor_get_abs_position (actor, &actor_x, &actor_y);
      
      layout_x = event->x - actor_x;
      layout_y = event->y - actor_y;

      /* Convert the pixel position into a text byte index */
      layout = clutter_label_get_layout (CLUTTER_LABEL (self));
      if (pango_layout_xy_to_index (layout,
				    layout_x * PANGO_SCALE,
				    layout_y * PANGO_SCALE,
				    &index, NULL))
	{
	  /* Check whether that byte index is covered by any of the
	     URL matches */
	  for (i = 0; i < self->matches->len; i++)
	    {
	      TweetUrlLabelMatch *match;
	      match = &g_array_index (self->matches, TweetUrlLabelMatch, i);

	      if (index >= match->start && index < match->end)
		{
		  new_selected_match = i;
		  break;
		}
	    }
	}
    }
  
  if (new_selected_match != self->selected_match)
    {
      self->selected_match = new_selected_match;
      clutter_actor_queue_redraw (actor);
    }

  return CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)->motion_event
    ? (CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)
       ->motion_event (actor, event))
    : FALSE;
}

static gboolean
tweet_url_label_leave_event (ClutterActor *actor, ClutterCrossingEvent *event)
{
  TweetUrlLabel *self = TWEET_URL_LABEL (actor);
  
  if (self->selected_match != -1)
    {
      self->selected_match = -1;
      clutter_actor_queue_redraw (actor);
    }

  return CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)->leave_event
    ? (CLUTTER_ACTOR_CLASS (tweet_url_label_parent_class)
       ->leave_event (actor, event))
    : FALSE;
}

static gboolean
tweet_url_label_button_press_event (ClutterActor *actor,
				    ClutterButtonEvent *event)
{
  TweetUrlLabel *self = TWEET_URL_LABEL (actor);
  
  if (self->selected_match != -1 && event->button == 1)
    {
      PangoLayout *layout;
      const gchar *label_text;
      gint label_text_len;
      TweetUrlLabelMatch *match = &g_array_index (self->matches,
						  TweetUrlLabelMatch,
						  self->selected_match);

      /* Get the text of the URL from a substring of the label text */
      layout = clutter_label_get_layout (CLUTTER_LABEL (self));
      label_text = pango_layout_get_text (layout);
      label_text_len = strlen (label_text);
      /* Make sure the match range is still valid */
      if (match->start >= 0 && match->end >= match->start
	  && match->end <= label_text_len)
	{
	  gchar *url_text = g_strndup (label_text + match->start,
				       match->end - match->start);
	  tweet_show_url (NULL, url_text);
	  g_free (url_text);
	}

      return TRUE;
    }
  else
    return FALSE;
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

  /* If there is at least one URL then make sure the actor is reactive
     so we can detect when the cursor moves over it */
  if (self->matches->len > 0)
    clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);

  /* We no longer know if the mouse is over the current URL */
  self->selected_match = -1;
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

static GdkCursor *
tweet_url_label_get_cursor (TweetHotActor *actor,
			    GdkDisplay    *display,
			    int            x,
			    int            y)
{
  TweetUrlLabel *self = TWEET_URL_LABEL (actor);
  
  /* If there is no URL selected then use the default cursor */
  if (self->selected_match == -1)
    return NULL;

  /* Don't create a new cursor if we have a cached one for the same
     display */
  if (self->hand_cursor == NULL
      || gdk_cursor_get_display (self->hand_cursor) != display)
    {
      /* Create a new hand cursor */
      if (self->hand_cursor)
	gdk_cursor_unref (self->hand_cursor);
      
      self->hand_cursor = gdk_cursor_new_for_display (display, GDK_HAND2);
    }

  return self->hand_cursor;
}

static void
tweet_url_label_dispose (GObject *object)
{
  TweetUrlLabel *self = (TweetUrlLabel *) object;

  if (self->url_regex)
    {
      g_regex_unref (self->url_regex);
      self->url_regex = NULL;
    }

  if (self->hand_cursor)
    {
      gdk_cursor_unref (self->hand_cursor);
      self->hand_cursor = NULL;
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
