NULL =

INCLUDES = \
	-DPREFIX=\""$(prefix)"\"				\
	-DLIBDIR=\""$(libdir)"\"				\
	-DDATADIR=\""$(datadir)"\"				\
	-DPKGDATADIR=\""$(datadir)/tweet"\"			\
	-DUIDATADIR=\""$(datadir)/gnome-2.0/ui"\"		\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	$(NULL)

common_cflags = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/twitter-glib \
	-I$(top_srcdir)/copy-and-paste \
	$(TWEET_CFLAGS)
	$(NULL)

common_ldadd = \
	$(top_builddir)/twitter-glib/libtwitter-glib-1.0.la \
	$(top_builddir)/copy-and-paste/tidy/libtidy.la \
	$(TWEET_LIBS) \
	$(NULL)


noinst_LTLIBRARIES = libtweet-ui.la
bin_PROGRAMS = tweet

libtweet_ui_la_SOURCES = \
	tweet-app.c \
	tweet-app.h \
	tweet-config.c \
	tweet-config.h \
	tweet-spinner.c \
	tweet-spinner.h \
	tweet-status-column.c \
	tweet-status-column.h \
	tweet-status-model.c \
	tweet-status-model.h \
	tweet-status-renderer.c \
	tweet-status-renderer.h \
	tweet-status-view.c \
	tweet-status-view.h \
	tweet-utils.c \
	tweet-utils.h \
	tweet-window.c \
	tweet-window.h \
	$(NULL)
libtweet_ui_la_CFLAGS = $(common_cflags)
libtweet_ui_la_LIBADD = $(common_ldadd)

tweet_SOURCES = tweet-main.c
tweet_CFLAGS = $(common_cflags)
tweet_LDADD = $(common_ldadd) libtweet-ui.la
