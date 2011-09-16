ACLOCAL_AMFLAGS = -I m4

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

a4include_HEADERS = src/a4/application.h
a4python_PYTHON = 

LDADD=liba4process.la $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_SYSTEM_LIBS)
AM_CPPFLAGS=$(BOOST_PROGRAM_OPTIONS_CPPFLAGS) $(BOOST_SYSTEM_CPPFLAGS)

lib_LTLIBRARIES=liba4process.la
liba4process_la_SOURCES=src/hash_lookup.cpp #src/application.cpp
#liba4process_la_LIBADD=$(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_SYSTEM_LIBS)
#liba4process_la_CPPFLAGS=$(BOOST_PROGRAM_OPTIONS_CPPFLAGS) $(BOOST_SYSTEM_CPPFLAGS)

## Test programs
check_PROGRAMS = test_hashlookup test_hashlookup_speed test_app
test_hashlookup_SOURCES = src/test_hashlookup.cpp
test_hashlookup_speed_SOURCES = src/test_hashlookup_speed.cpp
test_app_SOURCES = src/test_app.cpp

TESTS=$(check_PROGRAMS)

installcheck-local: bin/test_rw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_app

