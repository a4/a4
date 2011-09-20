ACLOCAL_AMFLAGS = -I m4
include $(srcdir)/m4/a4.am

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4
athenadatadir=${pkgdatadir}/athena

dist_python_SCRIPTS=python/grl2datasets python/grl2xml python/grl_and python/grl_diff python/id2xs python/ids2datasets python/mc_count python/mc_merge python/xml2grl
dist_athenadata_SCRIPTS=athena/aod2a4.py athena/gangatask.py athena/run.py
a4include_HEADERS = src/a4/grl.h
a4python_PYTHON = python/root2a4.py

A4PACK=atlas
include protobuf.make

AM_CPPFLAGS=$(PROTOBUF_CFLAGS)
LDADD=liba4atlas.la $(PROTOBUF_LIBS)

lib_LTLIBRARIES=liba4atlas.la
nodist_liba4atlas_la_SOURCES=$(PROTOBUF_H) $(PROTOBUF_CC)
dist_liba4atlas_la_SOURCES=src/a4/grl.h src/grl.cpp
liba4atlas_la_LIBADD=$(PROTOBUF_LIBS)

## Test programs
check_PROGRAMS = test_grl
test_grl_SOURCES = src/test_grl.cpp

TESTS=$(check_PROGRAMS)

