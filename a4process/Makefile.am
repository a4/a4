ACLOCAL_AMFLAGS = -I m4

A4PACK=process

include m4/a4.am
include protobuf.am

AM_CXXFLAGS+= -Wall -Werror

# Files to distribute
a4include_HEADERS = \
    src/a4/storable.h \
    src/a4/a4process.h src/a4/processor.h src/a4/driver.h src/a4/application.h \
    src/a4/hash_lookup.h src/a4/hash_lookup_impl.h\
    src/a4/object_store.h src/a4/object_store_impl.h\
    src/a4/results_processor.h

# Compilation options
LDADD=liba4process.la $(A4_A4IO_LIBS) $(PROTOBUF_LIBS) $(BOOST_LIBS)
AM_CPPFLAGS+=$(A4_A4IO_CPPFLAGS) $(PROTOBUF_CFLAGS) $(BOOST_CPPFLAGS)

# Main a4process library
lib_LTLIBRARIES=liba4process.la
nodist_liba4process_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H)
liba4process_la_SOURCES= $(a4include_HEADERS)\
    src/storable.cpp \
    src/processor.cpp \
    src/hash_lookup.cpp \
    src/object_store.cpp \
    src/driver.cpp
liba4process_la_LIBADD=$(A4_A4IO_LIBS) $(PROTOBUF_LIBS) $(BOOST_LIBS)


## Test programs
plain_test_programs=test_hashlookup test_hashlookup_speed test_objstore_speed test_storable test_histo
check_PROGRAMS = $(plain_test_programs) test_app
test_hashlookup_SOURCES = src/test_hashlookup.cpp
test_hashlookup_speed_SOURCES = src/test_hashlookup_speed.cpp
test_objstore_speed_SOURCES = src/test_objstore_speed.cpp
test_storable_SOURCES = src/test_storable.cpp
test_app_SOURCES = src/test_app.cpp
test_histo_SOURCES = src/test_histo.cpp

CLEANFILES+=test_histos.a4 test_storable.a4

dist_check_SCRIPTS = src/test_app.sh

TESTS=$(plain_test_programs) src/test_app.sh

