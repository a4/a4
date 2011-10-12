ACLOCAL_AMFLAGS = -I m4

include m4/a4.am

# Build . before src so that our all-local and clean-local hooks kicks in at
# the right time.
SUBDIRS = . a4io a4process a4hist a4atlas a4root

# Always include gtest in distributions.
DIST_SUBDIRS = gtest $(SUBDIRS)


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


# We would like to clean gtest when "make clean" is invoked.  But we have to
# be careful because clean-local is also invoked during "make distclean", but
# "make distclean" already recurses into gtest because it's listed among the
# DIST_SUBDIRS.  distclean will delete gtest/Makefile, so if we then try to
# cd to the directory again and "make clean" it will fail.  So, check that the
# Makefile exists before recursing.
clean-local:
	@if test -e gtest/Makefile; then \
	  echo "Making clean in gtest"; \
	  cd gtest && $(MAKE) $(AM_MAKEFLAGS) clean; \
	fi
