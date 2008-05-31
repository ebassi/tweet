SUBDIRS = icons

tweetuidir = $(datadir)/tweet
tweetui_DATA = tweet.ui tweet-preferences.xml

tweetappdir = $(datadir)/applications
tweetapp_in_files = tweet.desktop.in
tweetapp_DATA = $(tweetapp_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(tweetapp_in_files) $(tweetui_DATA)

CLEANFILES = $(tweetapp_DATA)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/tweet/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

