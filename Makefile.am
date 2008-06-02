SUBDIRS = twitter-glib tests copy-and-paste src po data

if ENABLE_GTK_DOC
SUBDIRS += doc
endif

DIST_SUBDIRS = $(SUBDIRS) doc

dist-hook:
	git-log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in
