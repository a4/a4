ACLOCAL_AMFLAGS = -I m4
 
edit = sed -e 's|$${prefix}|$(prefix)|g'
 
this_a4.sh: this_a4.sh.in
	rm -f $@ $@.tmp
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	$(edit) $${srcdir}$@.in > $@

bin_SCRIPTS = this_a4.sh

SUBDIRS = $(PROTOBUF_BUILD) a4io
