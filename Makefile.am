ACLOCAL_AMFLAGS = -I m4
 
edit = sed -e 's|$${prefix}|$(prefix)|g'
 
this_a4.sh: this_a4.sh.in
	rm -f $@ $@.tmp
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	$(edit) $${srcdir}$@.in > $@

install-data-local:
	if test "x$(DESTDIR)" != "x"; then DESTDIROPT="--root=$(DESTDIR)"; fi
	if test "x$(PROTOBUF_BUILD)" != "x"; then \
	    cd $(srcdir)/$(PROTOBUF_BUILD)/python;\
	    PYTHONPATH=$(PYTHONPATH):$(prefix)/lib/python2.6/site-packages $(PYTHON) setup.py install --prefix=$(prefix) $(DESTDIROPT); \
	fi
	
install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) `which $(PYTHON)` python || true


bin_SCRIPTS = this_a4.sh

SUBDIRS = $(PROTOBUF_BUILD) a4io
