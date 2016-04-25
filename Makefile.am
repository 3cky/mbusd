## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

mbusddocdir = ${prefix}/doc/mbusd
mbusddoc_DATA = \
	README.md\
	CHANGELOG.md\
	LICENSE

EXTRA_DIST = $(mbusddoc_DATA)

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

