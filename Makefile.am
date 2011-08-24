ACLOCAL_AMFLAGS = -I m4
 
edit = sed -e 's|$${prefix}|$(prefix)|g'
 
a4env.sh: a4env.sh.in
	rm -f $@ $@.tmp
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	$(edit) $${srcdir}$@.in > $@

bin_SCRIPTS = a4env.sh

SUBDIRS = $(PROTOBUF_BUILD) a4io
