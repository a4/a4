ACLOCAL_AMFLAGS = -I m4

A4PACK=root

include m4/a4.am
include protobuf.am

pkgdata_PROGRAMS=test_rw 
a4include_HEADERS = src/a4/a4io.h src/a4/exceptions.h src/a4/types.h \
    src/a4/string.h src/a4/string_impl.h src/a4/register.h src/a4/message.h \
    src/a4/input_stream.h src/a4/output_stream.h src/a4/input.h src/a4/output.h

# Configuration options
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(CERN_ROOT_SYSTEM_CFLAGS) $(PROTOBUF_CFLAGS) $(SNAPPY_CPPFLAGS) $(BOOST_CPPFLAGS)
LDADD=$(A4_A4IO_LIBS) $(CERN_ROOT_SYSTEM_LIBS) $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

## Test programs
check_PROGRAMS = test_rw
test_rw_SOURCES = $(PROTOBUF_H) src/test_rw.cpp

TESTS=$(check_PROGRAMS)




