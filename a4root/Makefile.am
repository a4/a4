ACLOCAL_AMFLAGS = -I m4

A4PACK=root

include m4/a4.am
include protobuf.am

# Configuration options
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(A4_A4PROCESS_CPPFLAGS) $(A4_A4HIST_CPPFLAGS) $(CERN_ROOT_SYSTEM_CFLAGS) $(PROTOBUF_CFLAGS) $(SNAPPY_CPPFLAGS) $(BOOST_CPPFLAGS)
LDADD=$(A4_A4IO_LIBS) $(A4_A4PROCESS_LIBS) $(A4_A4HIST_LIBS) $(CERN_ROOT_SYSTEM_LIBS) $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

bin_PROGRAMS=a4results2root
a4results2root_SOURCES = src/a4results2root.cpp

## Test programs
check_PROGRAMS = rwtest agnostic_dump
nodist_rwtest_SOURCES = $(PROTOBUF_CC) $(PROTOBUF_H)
rwtest_SOURCES = src/rwtest.cpp
agnostic_dump_SOURCES = src/agnostic_dump.cpp

#TESTS=$(check_PROGRAMS)




