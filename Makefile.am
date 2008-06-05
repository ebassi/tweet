SUBDIRS = twitter-glib tests copy-and-paste src po data

EXTRA_DIST = NEWS

dist-hook:
	git-log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in
