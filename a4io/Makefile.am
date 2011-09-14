ACLOCAL_AMFLAGS = -I m4

include $(srcdir)/m4/a4.am

A4PACK=io

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4
a4pythonprotodir=${pythondir}/a4/proto

dist_bin_SCRIPTS = python/a4dump python/a4info
dist_pkgdata_SCRIPTS=test_rw test_read python/test_pyrw
pkgdata_PROGRAMS=test_read test_read_nomd test_write_fw test_write_bw test_bigwrite test_write_nomd_fw test_write_nomd_bw
a4include_HEADERS = src/a4/a4io.h src/a4/stream.h src/a4/input_stream.h src/a4/output_stream.h src/a4/input.h src/a4/output.h
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
liba4io_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H) \
    src/gzip_stream.h src/gzip_stream.cpp \
    src/a4/input_stream.h src/input_stream.cpp \
    src/a4/output_stream.h src/output_stream.cpp \
    src/a4/stream.h src/stream.cpp \
    src/a4/input.h src/input.cpp \
    src/a4/output.h src/output.cpp
liba4io_la_LIBADD=$(PROTOBUF_LIBS)
liba4io_la_CPPFLAGS=$(PROTOBUF_CFLAGS)

## Test programs
check_PROGRAMS = test_rw test_io
test_bigwrite_SOURCES = $(PROTOBUF_H) src/test_bigwrite.cpp
test_rw_SOURCES = $(PROTOBUF_H) src/test_rw.cpp
test_io_SOURCES = $(PROTOBUF_H) src/test_io.cpp
test_read_SOURCES = $(PROTOBUF_H) src/test_read.cpp
test_read_nomd_SOURCES = $(PROTOBUF_H) src/test_read_nomd.cpp
test_write_fw_SOURCES = $(PROTOBUF_H) src/test_write_fw.cpp
test_write_bw_SOURCES = $(PROTOBUF_H) src/test_write_bw.cpp
test_write_nomd_fw_SOURCES = $(PROTOBUF_H) src/test_write_nomd_fw.cpp
test_write_nomd_bw_SOURCES = $(PROTOBUF_H) src/test_write_nomd_bw.cpp

TESTS=$(check_PROGRAMS)

installcheck-local: python/test_pyrw test_read test_write_fw test_write_bw
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
	./test_write_fw  
	./test_write_bw 
	./test_write_nomd_fw 
	./test_write_nomd_bw 
	./test_read test_fw.a4 
	./test_read test_bw.a4 
	./test_read_nomd test_nomd_fw.a4 
	./test_read_nomd test_nomd_bw.a4 
	cat test_fw.a4 test_fw.a4 > test_fwfw.a4 
	cat test_fw.a4 test_bw.a4 > test_fwbw.a4 
	cat test_bw.a4 test_fw.a4 > test_bwfw.a4 
	cat test_bw.a4 test_bw.a4 > test_bwbw.a4 
	./test_read test_fwfw.a4 
	./test_read test_fwbw.a4 
	./test_read test_bwfw.a4 
	./test_read test_bwbw.a4
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_fw.a4",1000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_bw.a4",1000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_fwfw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_fwbw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_bwfw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.A4Stream import test_read; import sys; sys.exit(test_read("test_bwbw.a4",2000))'
	rm -f test_fw.a4 test_bw.a4 test_fwfw.a4 test_bwfw.a4 test_fwbw.a4 test_bwbw.a4
	rm -f pytest_fw.a4 pytest_bw.a4 pytest_fwfw.a4 pytest_bwfw.a4 pytest_fwbw.a4 pytest_bwbw.a4
	rm -f test_rw.a4
	rm -f test_nomd_fw.a4 test_nomd_bw.a4



