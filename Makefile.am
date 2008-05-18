SUBDIRS = twitter-glib tests copy-and-paste src

dist-hook:
	git-log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in
