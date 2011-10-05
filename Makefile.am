ACLOCAL_AMFLAGS = -I m4

include m4/a4.am

SUBDIRS=a4io a4process a4atlas

bin_SCRIPTS += this_a4.sh
CLEANFILES += this_a4.sh m4/liba4.pc
EXTRA_DIST+=$(srcdir)/tutorial/ $(srcdir)/doc/ m4/liba4.pc_in

nodist_pkgconfig_DATA = m4/liba4.pc

# Dummy library for linking (does this actually work?)
lib_LTLIBRARIES=liba4.la
liba4_la_SOURCES=common/dummy.cpp
liba4_la_LIBADD=a4atlas/liba4atlas.la a4process/liba4process.la a4io/liba4io.la

# Rules for generating documentation
doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"
