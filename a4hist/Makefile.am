ACLOCAL_AMFLAGS = -I m4

A4PACK=hist

include m4/a4.am
include protobuf.make

# Files to distribute
a4include_HEADERS = \
    src/a4/alorentzvector.h \
    src/a4/h1.h src/a4/h2.h \
    src/a4/cutflow.h

# Compilation options
LDADD=liba4hist.la $(A4_LIBS) $(PROTOBUF_LIBS) $(BOOST_LIBS)
AM_CPPFLAGS+=$(A4_CPPFLAGS) $(PROTOBUF_CFLAGS) $(BOOST_CPPFLAGS)

# Main a4process library
lib_LTLIBRARIES=liba4hist.la
nodist_liba4hist_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H)
liba4hist_la_SOURCES= \
    src/a4/h1.h src/h1.cpp \
    src/a4/h2.h src/h2.cpp \
    src/a4/cutflow.h src/cutflow.cpp
liba4hist_la_LIBADD=$(A4_LIBS) $(PROTOBUF_LIBS) $(BOOST_LIBS)

## Test programs
check_PROGRAMS = test_hist
test_hist_SOURCES = src/test_hist.cpp

TESTS=$(check_PROGRAMS)

