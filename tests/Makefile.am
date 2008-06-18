include $(top_srcdir)/Makefile.decl

NULL =

#noinst_PROGRAMS = $(TEST_PROGS)
noinst_PROGRAMS = test-user-timeline test-status-send

INCLUDES = -I$(top_srcdir)
progs_ldadd = $(top_builddir)/twitter-glib/libtwitter-glib-1.0.la $(TWITTER_GLIB_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/twitter-glib \
	$(TWITTER_GLIB_CFLAGS) \
	$(TWEET_DEBUG_CFLAGS)

AM_CFLAGS = -g

TESTS_ENVIRONMENT = srcdir=$(srcdir)

#TEST_PROGS                 += test-user-timeline
test_user_timeline_SOURCES  = test-user-timeline.c
test_user_timeline_LDADD    = $(progs_ldadd)

#TEST_PROGS               += test-status-send
test_status_send_SOURCES  = test-status-send.c
test_status_send_LDADD    = $(progs_ldadd)

