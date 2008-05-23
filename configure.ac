m4_define([tweet_major_version], [0])
m4_define([tweet_minor_version], [1])
m4_define([tweet_micro_version], [0])

m4_define([tweet_version],
          [tweet_major_version.tweet_minor_version.tweet_micro_version])

m4_define([tweet_interface_age], [0])
m4_define([tweet_binary_age],
          [m4_eval(100 * tweet_minor_version + tweet_micro_version)])

m4_define([lt_current],
          [m4_eval(100 * tweet_minor_version + tweet_micro_version - tweet_interface_age)])
m4_define([lt_revision], [tweet_interface_age])
m4_define([lt_age],
          [m4_eval(tweet_binary_age - tweet_interface_age)])

m4_define([glib_req_version], [2.16])
m4_define([json_glib_req_version], [0.6.0])
m4_define([soup_req_version], [2.4.1])
m4_define([clutter_req_version], [0.6.2])
m4_define([tidy_req_version], [0.0])
m4_define([gtk_req_version], [2.12])
m4_define([clutter_gtk_req_version], [0.6.0])
m4_define([clutter_cairo_req_version], [0.6.2])

AC_PREREQ([2.59])
AC_INIT([tweet], [tweet_version], [], [tweet])

AM_INIT_AUTOMAKE([1.10])
AM_CONFIG_HEADER([config.h])

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS
GNOME_DEBUG_CHECK

IT_PROG_INTLTOOL([0.35.0])

AM_DISABLE_STATIC
AM_PATH_GLIB_2_0
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h])
AC_C_CONST
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])
AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_CHECK_FUNCS([strptime localtime_r])

TWEET_MAJOR_VERSION=tweet_major_version
TWEET_MINOR_VERSION=tweet_minor_version
TWEET_MICRO_VERSION=tweet_micro_version
TWEET_VERSION=tweet_version
AC_SUBST(TWEET_MAJOR_VERSION)
AC_SUBST(TWEET_MICRO_VERSION)
AC_SUBST(TWEET_MINOR_VERSION)
AC_SUBST(TWEET_VERSION)

dnl twitter-glib checks
LT_CURRENT=lt_current
LT_REVISION=lt_revision
LT_AGE=lt_age
LT_VERSION="$LT_CURRENT:$LT_REVISION:$LT_AGE"
TWITTER_GLIB_LT_LDFLAGS="-versio-info $LT_VERSION"
AC_SUBST(TWITTER_GLIB_LT_LDFLAGS)

PKG_CHECK_MODULES(TWITTER_GLIB,
                  gobject-2.0 >= glib_req_version dnl
                  gio-2.0 >= glib_req_version dnl
                  json-glib-1.0 >= json_glib_req_version dnl
                  libsoup-2.4 >= soup_req_version dnl
                  gdk-pixbuf-2.0)
AC_SUBST(TWITTER_GLIB_CFLAGS)
AC_SUBST(TWITTER_GLIB_LIBS)

dnl tweet checks
PKG_CHECK_MODULES(TWEET,
                  gobject-2.0 >= glib_req_version dnl
                  json-glib-1.0 >= json_glib_req_version dnl
                  clutter-0.6 >= clutter_req_version dnl
                  gtk+-2.0 >= gtk_req_version dnl
                  clutter-gtk-0.6 >= clutter_gtk_req_version dnl
                  clutter-cairo-0.6 >= clutter_cairo_req_version)
AC_SUBST(TWEET_CFLAGS)
AC_SUBST(TWEET_LIBS)

dnl = Enable debug level ===================================================

m4_define([debug_default],
          m4_if(m4_eval(tweet_minor_version % 2), [1], [yes], [minimum]))

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                             [turn on debugging @<:@default=debug_default@:>@]),
,
              enable_debug=debug_default)

if test "x$enable_debug" = "xyes"; then
  test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
  TWEET_DEBUG_CFLAGS="-DTWEET_ENABLE_DEBUG"
else
  if test "x$enable_debug" = "xno"; then
    TWEET_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
  else # minimum
    TWEET_DEBUG_CFLAGS="-DTWEET_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
  fi
fi

AC_SUBST(TWEET_DEBUG_CFLAGS)

dnl = Enable strict compiler flags =========================================

# use strict compiler flags only on development releases
#m4_define([maintainer_flags_default],
#          m4_if(m4_eval(tweet_minor_version % 2), [1], [yes], [no]))
m4_define([maintainer_flags_default], [no])
AC_ARG_ENABLE([maintainer-flags],
              AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                             [Use strict compiler flags @<:@default=maintainer_flags_default@:>@]),,
              enable_maintainer_flags=maintainer_flags_default)

if test "x$enable_maintainer_flags" = "xyes"; then
  CPPFLAGS="$CPPFLAGS -g -Wall -Wshadow -Wcast-align -Wno-uninitialized -Werror"
else
  CPPFLAGS="$CPPFLAGS -g -Wall"
fi

dnl Internationalization
GETTEXT_PACKAGE=tweet
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE", [Define the gettext package to use])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

dnl gnome-doc-utils stuff
GNOME_DOC_INIT

AC_CONFIG_FILES([
        Makefile
        twitter-glib/Makefile
        twitter-glib/twitter-version.h
        twitter-glib/twitter-glib.pc
        copy-and-paste/Makefile
        copy-and-paste/tidy/Makefile
        src/Makefile
        tests/Makefile
        po/Makefile.in
])

AC_OUTPUT

echo ""
echo " Tweet $VERSION"
echo ""
echo "         Prefix: ${prefix}"
echo "    Debug level: ${enable_debug}"
echo " Compiler flags: ${CPPFLAGS}"
echo ""
