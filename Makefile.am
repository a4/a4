ACLOCAL_AMFLAGS = -I m4
 
edit = sed -e 's|$${prefix}|$(prefix)|g'
 
a4env.sh: a4env.sh.in
	rm -f $@ $@.tmp
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	$(edit) $${srcdir}$@.in > $@

EXTRA_DIST = \
	a4env.sh

bin_SCRIPTS = a4env.sh


SUBDIRS = $(PROTOBUF_BUILD) a4io

#bin_PROGRAMS = arm
#arm_SOURCES = arm.c
## link with the hand library.
#arm_LDADD = hand/libhand.a
