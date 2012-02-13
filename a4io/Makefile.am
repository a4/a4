ACLOCAL_AMFLAGS = -I m4

A4PACK=io

include m4/a4.am
include protobuf.am

# Files to distribute
dist_bin_SCRIPTS = python/a4dump.py python/a4info.py
dist_pkgdata_SCRIPTS=python/test_pyrw
bin_PROGRAMS = test_read test_read test_read_nomd test_write_fw test_write_bw test_bigwrite test_write_nomd_fw test_write_nomd_bw
a4include_HEADERS = src/a4/a4io.h src/a4/exceptions.h src/a4/types.h src/a4/debug.h \
    src/a4/string.h src/a4/string_impl.h src/a4/register.h src/a4/message.h \
    src/a4/input_stream.h src/a4/output_stream.h src/a4/input.h src/a4/output.h
nodist_a4include_HEADERS = src/a4/config.h
a4python_PYTHON = python/a4/__init__.py python/a4/stream.py python/a4/snappy_stream.py python/a4/zlib_stream.py python/a4/proto_class_pool.py

# Configuration options
AM_CPPFLAGS+=$(PROTOBUF_CFLAGS) $(SNAPPY_CPPFLAGS) $(BOOST_CPPFLAGS)
LDADD=liba4io.la $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

# Main a4io library
LIBRARY_SOURCES=$(a4include_HEADERS) \
    src/message.cpp \
    src/zero_copy_resource.h src/zero_copy_resource.cpp \
    src/proto_class_pool.h src/proto_class_pool.cpp \
    src/dynamic_message.h src/dynamic_message.cpp \
    src/input_stream_impl.h src/input_stream_impl.cpp \
    src/base_compressed_streams.h src/string.cpp \
    src/gzip_stream.h src/gzip_stream.cpp \
    src/snappy_stream.h src/snappy_stream.cpp \
    src/input_stream.cpp src/output_stream.cpp \
    src/register.cpp src/input.cpp src/output.cpp \
    src/dynamic_message.h src/exceptions.cpp \
    src/remote_io.h src/remote_io.cpp
lib_LTLIBRARIES=liba4io.la
nodist_liba4io_la_SOURCES=$(PROTOBUF_CC) $(PROTOBUF_H)
dist_liba4io_la_SOURCES=$(LIBRARY_SOURCES)
liba4io_la_LIBADD=-ldl $(PROTOBUF_LIBS) $(SNAPPY_LDFLAGS) $(BOOST_LIBS)

bin_PROGRAMS += a4dump a4info
a4dump_SOURCES = src/apps/a4dump.cpp
a4info_SOURCES = src/apps/a4info.cpp

## Test programs
check_PROGRAMS = test_io test_thread gtests
test_bigwrite_SOURCES = src/tests/test_bigwrite.cpp
test_io_SOURCES = src/tests/test_io.cpp
test_thread_SOURCES = src/tests/test_thread.cpp
test_read_SOURCES = src/tests/test_read.cpp
test_read_nomd_SOURCES = src/tests/test_read_nomd.cpp
test_write_fw_SOURCES = src/tests/test_write_fw.cpp
test_write_bw_SOURCES = src/tests/test_write_bw.cpp
test_write_nomd_fw_SOURCES = src/tests/test_write_nomd_fw.cpp
test_write_nomd_bw_SOURCES = src/tests/test_write_nomd_bw.cpp

nodist_gtests_SOURCES = $(GTEST_APP)
gtests_SOURCES = src/tests/test_rw.cpp src/tests/test_merge_metadata.cpp src/tests/test_remote_io.cpp src/remote_io.cpp src/remote_io.h
gtests_CPPFLAGS = $(GTEST_CPPFLAGS) $(AM_CPPFLAGS)
gtests_LDADD = $(LDADD) $(GTEST_LIBS)

TESTS=$(check_PROGRAMS)

tests: $(TESTS)

installcheck-local: python/test_pyrw test_read test_write_fw test_write_bw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_pyrw
	source $(bindir)/this_a4.sh && a4info.py -m pytest_fw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4info.py -m pytest_bw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump.py -a pytest_fw.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump.py -a pytest_bw.a4 > /dev/null
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
	ln -s test_fw.a4 test_fw1.a4
	ln -s test_bw.a4 test_bw1.a4
	cat test_fw.a4 test_fw1.a4 > test_fwfw.a4 
	cat test_bw.a4 test_bw1.a4 > test_bwbw.a4 
	cat test_fw.a4 test_bw.a4 > test_fwbw.a4 
	cat test_bw.a4 test_fw.a4 > test_bwfw.a4 
	rm test_fw1.a4 test_bw1.a4
	./test_read test_fwfw.a4 
	./test_read test_fwbw.a4 
	./test_read test_bwfw.a4 
	./test_read test_bwbw.a4
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fw.a4",1000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bw.a4",1000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwfw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwbw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwfw.a4",2000))'
	source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwbw.a4",2000))'
	rm -f test_fw.a4 test_bw.a4 test_fwfw.a4 test_bwfw.a4 test_fwbw.a4 test_bwbw.a4
	rm -f pytest_fw.a4 pytest_bw.a4 pytest_fwfw.a4 pytest_bwfw.a4 pytest_fwbw.a4 pytest_bwbw.a4
	rm -f test_rw.a4 test_mm.a4
	rm -f test_nomd_fw.a4 test_nomd_bw.a4
	rm -f test_thread.a4 test_io.a4



