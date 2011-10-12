ACLOCAL_AMFLAGS = -I m4

A4_GTEST_SUBDIR=gtest

include m4/a4.am

# Build . before everything else our all-local and 
# clean-local hooks kicks in at the right time.
SUBDIRS = . a4io a4process a4hist a4atlas a4root

nodist_pkgconfig_DATA = m4/liba4.pc
bin_SCRIPTS += this_a4.sh
CLEANFILES += this_a4.sh m4/liba4.pc
EXTRA_DIST+=$(srcdir)/tutorial/ $(srcdir)/doc/ m4/liba4.pc_in
EXTRA_DIST+=get_miniboost.sh get_protobuf.sh get_snappy.sh

# Rules for generating documentation
doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"



