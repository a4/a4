ACLOCAL_AMFLAGS = -I m4

SUBDIRS=

include $(srcdir)/m4/protobuf.am 
 
edit = sed -e 's|$${prefix}|$(prefix)|g'
 
this_a4.sh: this_a4.sh.in
	rm -f $@ $@.tmp
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	$(edit) $${srcdir}$@.in > $@

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) `which $(PYTHON)` python || true

bin_SCRIPTS = this_a4.sh

SUBDIRS+=a4io a4process
