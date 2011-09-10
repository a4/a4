ACLOCAL_AMFLAGS = -I m4

SUBDIRS=

include $(srcdir)/m4/protobuf.am
include $(srcdir)/m4/a4.am

SUBDIRS+=a4io a4process

doxygen-user:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/a4_user.doxygen

doxygen-dev:
	mkdir -p doc
	srcdir=$(srcdir) builddir=$(builddir) doxygen $(srcdir)/doc/a4_dev.doxygen

doc: doxygen-user doxygen-dev
	echo "generated user and developer docs"
