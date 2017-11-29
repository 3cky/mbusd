## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc

mbusddocdir = ${docdir}
mbusddoc_DATA = \
	README.md\
	CHANGELOG.md\
	LICENSE

mbusdconfdir = ${mbusddocdir}
mbusdconf_DATA = \
	conf/mbusd.conf.example

EXTRA_DIST = $(mbusddoc_DATA) $(mbusdconf_DATA)

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

# systemd
AM_DISTCHECK_CONFIGURE_FLAGS = \
    --with-systemdsystemunitdir='$$(prefix)/$(systemdsystemunitdir)'

if HAVE_SYSTEMD
systemdsystemunit_DATA = \
	systemd-units/mbusd@.service

$(systemdsystemunit_DATA):
	-$(SED) -e 's,@bindir\@,$(bindir),g' '$@.in' >'$@'

distclean-local::
	-$(RM) $(systemdsystemunit_DATA)
endif
