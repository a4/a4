ACLOCAL_AMFLAGS = -I m4

include $(srcdir)/m4/a4.am

A4PACK=io

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4
a4pythonprotodir=${pythondir}/a4/proto

dist_bin_SCRIPTS = bin/a4dump bin/a4info
dist_pkgdata_SCRIPTS=test_rw test_read bin/test_pyrw
pkgdata_PROGRAMS=test_bigwrite test_bigread 
a4include_HEADERS = src/a4/stream.h src/a4/input_stream.h src/a4/output_stream.h src/a4/a4io_config.h
a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py
a4pythonproto_PYTHON = python/a4/proto/__init__.py

protodir=${localstatedir}/a4/proto/$(A4PACK)
protoincludedir=${includedir}/a4/proto/$(A4PACK)
protopythondir=${pythondir}/a4/proto/$(A4PACK)

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

AM_CPPFLAGS=$(PROTOBUF_CFLAGS)
LDADD=liba4io.la $(PROTOBUF_LIBS)

include protobuf.make

dist_proto_DATA=$(PROTOBUF_PROTO)
protoinclude_HEADERS = $(PROTOBUF_H)
protopython_PYTHON = $(PROTOBUF_PY) $(PYDIR)/__init__.py

lib_LTLIBRARIES=liba4io.la
liba4io_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H) src/gzip_stream.h src/gzip_stream.cpp src/input_stream.cpp src/output_stream.cpp src/stream.cpp
liba4io_la_LIBADD=$(PROTOBUF_LIBS)
liba4io_la_CPPFLAGS=$(PROTOBUF_CFLAGS)

## Test programs
check_PROGRAMS = test_rw test_read test_write_fw test_write_bw 
test_bigwrite_SOURCES = $(PROTOBUF_H) src/test_bigwrite.cpp
test_bigread_SOURCES = $(PROTOBUF_H) src/test_bigread.cpp
test_rw_SOURCES = $(PROTOBUF_H) src/test_rw.cpp
test_read_SOURCES = $(PROTOBUF_H) src/test_read.cpp
test_write_fw_SOURCES = $(PROTOBUF_H) src/test_write_fw.cpp
test_write_bw_SOURCES = $(PROTOBUF_H) src/test_write_bw.cpp

TESTS=$(check_PROGRAMS)

installcheck-local: bin/test_pyrw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_pyrw
	source $(bindir)/this_a4.sh && a4info -m pytest_fw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4info -m pytest_bw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump -a pytest_fw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump -a pytest_bw.a4 > /dev/null
	./test_read pytest_fw.a4 > /dev/null
	./test_read pytest_bw.a4 > /dev/null
	./test_read pytest_fwfw.a4 > /dev/null
	./test_read pytest_bwfw.a4 > /dev/null
	./test_read pytest_fwbw.a4 > /dev/null
	./test_read pytest_bwbw.a4 > /dev/null
	./test_write wtest.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4info -m wtest.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump -a wtest.a4 > /dev/null
	rm test.a4 pytest*.a4 wtest.a4


