ACLOCAL_AMFLAGS = -I m4

include m4/a4.am

# Build . before src so that our all-local and clean-local hooks kicks in at
# the right time.
SUBDIRS = . a4io a4process a4atlas

# Always include gtest in distributions.
DIST_SUBDIRS = gtest $(SUBDIRS)


bin_SCRIPTS += this_a4.sh
CLEANFILES += this_a4.sh m4/liba4.pc
EXTRA_DIST+=$(srcdir)/tutorial/ $(srcdir)/doc/ m4/liba4.pc_in

nodist_pkgconfig_DATA = m4/liba4.pc

# Rules for generating documentation
doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"

# Build gtest before we build protobuf tests.  We don't add gtest to SUBDIRS
# because then "make check" would also build and run all of gtest's own tests,
# which takes a lot of time and is generally not useful to us.  Also, we don't
# want "make install" to recurse into gtest since we don't want to overwrite
# the installed version of gtest if there is one.
check-local:
	@echo "Making lib/libgtest.a lib/libgtest_main.a in gtest"
	@cd gtest && $(MAKE) $(AM_MAKEFLAGS) lib/libgtest.la lib/libgtest_main.la

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
