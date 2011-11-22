ACLOCAL_AMFLAGS = -I m4

A4PACK=atlas

include m4/a4.am
include protobuf.am

## What to dostribute and where
athenadatadir=${pkgdatadir}/athena
dist_python_SCRIPTS=python/grl2datasets python/grl2xml python/grl_and python/grl_diff python/id2xs python/ids2datasets python/mc_count python/mc_merge python/xml2grl
dist_athenadata_SCRIPTS=athena/gangatask.py
a4include_HEADERS = src/a4/grl.h
a4python_PYTHON = python/root2a4.py
bin_PROGRAMS = a4reweight
bin_SCRIPTS += python/grl2xml python/xml2grl python/grl_and python/grl_diff python/id2xs python/ids2datasets

## Compilation flags
AM_CPPFLAGS+=$(BOOST_CPPFLAGS) $(PROTOBUF_CFLAGS) $(A4_A4IO_CPPFLAGS) $(A4_A4PROCESS_CPPFLAGS) $(A4_A4HIST_CPPFLAGS)
LIBS=$(BOOST_LIBS) $(PROTOBUF_LIBS) $(A4_A4IO_LIBS) $(A4_A4PROCESS_LIBS) $(A4_A4HIST_LIBS)
LDADD=liba4atlas.la $(LIBS)

## Main a4atlas library
lib_LTLIBRARIES=liba4atlas.la
nodist_liba4atlas_la_SOURCES=$(PROTOBUF_H) $(PROTOBUF_CC)
dist_liba4atlas_la_SOURCES=src/a4/grl.h src/grl.cpp
liba4atlas_la_LIBADD=$(PROTOBUF_LIBS)

## Programs
a4reweight_SOURCES=src/a4reweight.cpp
#a4reweight_LDADD=$(LDADD)

## Tests programs
check_PROGRAMS = test_grl
test_grl_SOURCES = src/test_grl.cpp

TESTS=$(check_PROGRAMS)

