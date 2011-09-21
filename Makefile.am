ACLOCAL_AMFLAGS = -I m4

SUBDIRS=

include m4/protobuf.am
include m4/a4.am

SUBDIRS+=a4atlas a4io a4process

bin_SCRIPTS = this_a4.sh

doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"
