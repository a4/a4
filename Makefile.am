ACLOCAL_AMFLAGS = -I m4

SUBDIRS=

include m4/a4.am

SUBDIRS+=a4io a4process a4hist a4atlas

bin_SCRIPTS += this_a4.sh
CLEANFILES += this_a4.sh
EXTRA_DIST+=$(srcdir)/tutorial/ $(srcdir)/doc/

# Main a4process library
lib_LTLIBRARIES=liba4.la
liba4_la_SOURCES=common/dummy.cpp
liba4_la_LIBADD=a4atlas/liba4atlas.la a4process/liba4process.la a4io/liba4io.la

doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"
