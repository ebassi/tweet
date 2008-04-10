SUBDIRS = twitter-glib tests src

dist-hook:
	git-log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in
