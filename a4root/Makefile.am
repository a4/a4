ACLOCAL_AMFLAGS = -I m4

A4PACK=root

include m4/a4.am
include protobuf.am

pkgdata_PROGRAMS=rwtest

# Configuration options
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(CERN_ROOT_SYSTEM_CFLAGS) $(PROTOBUF_CFLAGS) $(SNAPPY_CPPFLAGS) $(BOOST_CPPFLAGS)
LDADD=$(A4_A4IO_LIBS) $(CERN_ROOT_SYSTEM_LIBS) $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

## Test programs
check_PROGRAMS = rwtest
rwtest_SOURCES = $(PROTOBUF_CC) $(PROTOBUF_H) src/rwtest.cpp

TESTS=$(check_PROGRAMS)




