tweetappdir = $(datadir)/applications
tweetapp_in_files = tweet.desktop.in
tweetapp_DATA = $(tweetapp_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(tweetapp_in_files)

CLEANFILES = $(tweetapp_DATA)
