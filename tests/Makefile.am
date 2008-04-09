include $(top_srcdir)/Makefile.decl

NULL =

noinst_PROGRAMS = $(TEST_PROGS)

INCLUDES = -I$(top_srcdir) $(TWITTER_GLIB_CFLAGS)
progs_ldadd = $(top_builddir)/twitter-glib/libtwitter-glib-1.0.la $(TWITTER_GLIB_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/twitter-glib \
	$(TWEET_DEBUG_CFLAGS)

AM_CFLAGS = -g

TESTS_ENVIRONMENT = srcdir=$(srcdir)

TEST_PROGS                   += test-public-timeline
test_public_timeline_SOURCES  = test-public-timeline.c
test_public_timeline_LDADD    = $(progs_ldadd)

