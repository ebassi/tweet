SUBDIRS = twitter-glib tests copy-and-paste src po data

EXTRA_DIST = 			\
	NEWS 			\
	TODO 			\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in 	\
	omf.make 		\
	xmldocs.make 		\
	gnome-doc-utils.make

DISTCLEANFILES =		\
	gnome-doc-utils.make	\
	intltool-extract	\
	intltool-merge		\
	intltool-update

dist-hook:
	git-log --stat > ChangeLog.in && \
	cp -f ChangeLog.in $(distdir)/ChangeLog && \
	rm -f ChangeLog.in
