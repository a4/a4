ACLOCAL_AMFLAGS = -I m4

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

a4include_HEADERS = \
    src/a4/a4process.h src/a4/driver.h src/a4/appdrivers.h \
    src/a4/hash_lookup.h src/a4/hash_lookup_impl.h\
    src/a4/object_store.h src/a4/object_store_impl.h

a4python_PYTHON = 

LDADD=liba4process.la $(A4_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_SYSTEM_LIBS)
AM_CPPFLAGS=$(A4_CPPFLAGS) $(BOOST_PROGRAM_OPTIONS_CPPFLAGS) $(BOOST_SYSTEM_CPPFLAGS)

lib_LTLIBRARIES=liba4process.la
liba4process_la_SOURCES= \
    src/a4/a4process.h src/a4process.cpp \
    src/a4/hash_lookup.h src/a4/hash_lookup_impl.h src/hash_lookup.cpp \
    src/a4/object_store.h src/a4/object_store_impl.h src/object_store.cpp \
    src/a4/driver.h src/a4/appdrivers.h src/driver.cpp

#liba4process_la_LIBADD=$(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_SYSTEM_LIBS)
#liba4process_la_CPPFLAGS=$(BOOST_PROGRAM_OPTIONS_CPPFLAGS) $(BOOST_SYSTEM_CPPFLAGS)

## Test programs
check_PROGRAMS = test_hashlookup test_hashlookup_speed test_objstore_speed test_app
test_hashlookup_SOURCES = src/test_hashlookup.cpp
test_hashlookup_speed_SOURCES = src/test_hashlookup_speed.cpp
test_objstore_speed_SOURCES = src/test_objstore_speed.cpp
test_app_SOURCES = src/test_app.cpp
test_app_CPPFLAGS = $(AM_CPPFLAGS) $(A4_CPPFLAGS) $(PROTOBUF_CFLAGS) -I$(builddir)/../a4io/src
test_app_LDADD=$(LDADD) $(A4_LIBS)

check_SCRIPTS = src/test_app.sh

TESTS=test_hashlookup test_hashlookup_speed test_objstore_speed src/test_app.sh

installcheck-local: bin/test_rw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_app

