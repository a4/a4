ACLOCAL_AMFLAGS = -I m4

A4PACK=hist

include m4/a4.am
include protobuf.am

# Files to distribute
a4include_HEADERS = \
    src/a4/alorentzvector.h \
    src/a4/axis.h \
    src/a4/histogram.h \
    src/a4/cutflow.h \
    src/a4/a4hist.h \
    $(PROTOBUF_H)

# Compilation options
LIBS=$(A4_A4IO_LIBS) $(A4_A4PROCESS_LIBS) $(PROTOBUF_LIBS) $(BOOST_LIBS)
LDADD=liba4hist.la $(LIBS)
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(A4_A4PROCESS_CPPFLAGS) $(PROTOBUF_CFLAGS) $(BOOST_CPPFLAGS)

# Main a4process library
lib_LTLIBRARIES=liba4hist.la
nodist_liba4hist_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H)
liba4hist_la_SOURCES=$(a4include_HEADERS) src/axis.cpp src/h1.cpp src/h2.cpp src/cutflow.cpp
liba4hist_la_LIBADD=$(LIBS)

## Test programs
check_PROGRAMS = gtest
nodist_gtest_SOURCES = $(GTEST_APP)
gtest_SOURCES = src/test_hist.cpp
gtest_CPPFLAGS = $(GTEST_CPPFLAGS) $(AM_CPPFLAGS)
gtest_LDADD = $(LDADD) $(GTEST_LIBS)

TESTS=$(check_PROGRAMS)

