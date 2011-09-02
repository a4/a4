ACLOCAL_AMFLAGS = -I m4

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

a4include_HEADERS = src/a4/application.h
a4python_PYTHON = 

LDADD=liba4_process.la

lib_LTLIBRARIES=liba4_process.la
liba4_process_la_SOURCES=src/application.cpp
liba4_process_la_LIBADD=$(BOOST_PROGRAM_OPTIONS_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIBS)
liba4_process_la_CPPFLAGS=$(BOOST_PROGRAM_OPTIONS_CPPFLAGS)

## Test programs
check_PROGRAMS = test_app
test_app_SOURCES = src/test_app.cpp

TESTS=$(check_PROGRAMS)

installcheck-local: bin/test_rw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_app

