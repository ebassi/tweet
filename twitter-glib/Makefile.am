include $(top_srcdir)/Makefile.decl

NULL =

INCLUDES = \
	-I$(top_srcdir)			\
	-DPREFIX=\""$(prefix)"\"	\
	-DLIBDIR=\""$(libdir)"\"	\
	-DG_DISABLE_DEPRECATED		\
	-DG_LOG_DOMAIN=\"Twitter\"	\
	$(TWITTER_GLIB_CFLAGS)		\
	$(TWEET_DEBUG_CFLAGS)		\
	$(NULL)

LDADD = $(TWITTER_GLIB_LT_LDFLAGS) -export-dynamic -rpath $(libdir)

MARSHALFILES = twitter-marshal.c twitter-marshal.h
ENUMFILES = twitter-enum-types.c twitter-enum-types.h
STAMPFILES = stamp-twitter-marshal.h stamp-twitter-enum-types.h

BUILT_SOURCES = $(MARSHALFILES) $(ENUMFILES)

sources_public_h = \
	$(top_srcdir)/twitter-glib/twitter-common.h \
	$(top_srcdir)/twitter-glib/twitter-client.h \
	$(top_srcdir)/twitter-glib/twitter-status.h \
	$(top_srcdir)/twitter-glib/twitter-timeline.h \
	$(top_srcdir)/twitter-glib/twitter-user.h \
	$(top_srcdir)/twitter-glib/twitter-user-list.h \
	$(NULL)

sources_private_h = \
	$(top_srcdir)/twitter-glib/twitter-api.h \
	$(top_srcdir)/twitter-glib/twitter-private.h \
	$(NULL)

sources_c = \
	twitter-api.c \
	twitter-common.c \
	twitter-client.c \
	twitter-status.c \
	twitter-timeline.c \
	twitter-user.c \
	twitter-user-list.c \
	$(NULL)

twitter-marshal.h: stamp-twitter-marshal.h
	@true
stamp-twitter-marshal.h: twitter-marshal.list
	$(GLIB_GENMARSHAL) \
		--prefix=_twitter_marshal \
		--header \
	  $(top_srcdir)/twitter-glib/twitter-marshal.list > xgen-cmh \
	&& (cmp -s xgen-cmh twitter-marshal.h || cp xgen-cmh twitter-marshal.h ) \
	&& rm -f xgen-cmh \
	&& echo timestamp > $(@F)

twitter-marshal.c: twitter-marshal.h Makefile
	( echo "#include \"twitter-marshal.h\"" ; \
	  $(GLIB_GENMARSHAL) \
	  	--prefix=_twitter_marshal \
		--body \
	  $(top_srcdir)/twitter-glib/twitter-marshal.list --body ) >> xgen-cmc \
	&& cp xgen-cmc twitter-marshal.c \
	&& rm -f xgen-cmc

twitter-enum-types.h: stamp-twitter-enum-types.h
	@true
stamp-twitter-enum-types.h: $(sources_public_h) Makefile
	( cd $(top_srcdir)/twitter-glib && \
	  $(GLIB_MKENUMS) \
	  	--template $(top_srcdir)/twitter-glib/twitter-enum-types.h.in \
	  $(sources_public_h) ) > xgen-ceth \
	&& (cmp xgen-ceth twitter-enum-types.h || cp xgen-ceth twitter-enum-types.h) \
	&& rm -f xgen-ceth \
	&& echo timestamp > $(@F)

twitter-enum-types.c: twitter-enum-types.h
	( cd $(top_srcdir)/twitter-glib && \
	  $(GLIB_MKENUMS) \
	  	--template $(top_srcdir)/twitter-glib/twitter-enum-types.c.in \
	  $(sources_public_h) ) > xgen-cetc \
	&& cp xgen-cetc twitter-enum-types.c \
	&& rm -f xgen-cetc

lib_LTLIBRARIES = libtwitter-glib-1.0.la

libtwitter_glib_1_0_la_LIBADD = $(TWITTER_GLIB_LIBS)
libtwitter_glib_1_0_la_SOURCES = \
	$(sources_public_h) \
	$(sources_private_h) \
	$(sources_c) \
	$(BUILT_SOURCES) \
	$(NULL)
libtwitter_glib_1_0_la_LDFLAGS = $(LDADD)

twitterincludedir = $(includedir)/twitter-glib-1.0/twitter-glib
twitterinclude_DATA = \
	$(sources_public_h) \
	$(top_srcdir)/twitter-glib/twitter-enum-types.h \
	$(top_srcdir)/twitter-glib/twitter-version.h \
	$(top_srcdir)/twitter-glib/twitter-glib.h \
	$(NULL)

pcfiledir = $(libdir)/pkgconfig
pcfile_DATA = twitter-glib-1.0.pc

twitter-glib-1.0.pc: twitter-glib.pc
	@cp -f $< $@

CLEANFILES = $(STAMPFILES) $(BUILT_SOURCES) $(pcfile_DATA)

DISTCLEANFILES = \
	$(ENUMFILES) \
	$(MARSHALFILES) \
	twitter-version.h \
	$(NULL)

EXTRA_DIST += \
	twitter-version.h.in \
	twitter-glib.h \
	twitter-marshal.list \
	twitter-enum-types.h.in \
	twitter-enum-types.c.in \
	$(NULL)

