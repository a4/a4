ACLOCAL_AMFLAGS = -I m4

A4PACK=io

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

bin_SCRIPTS=bin/a4pool bin/a4dump bin/a4info
a4include_HEADERS = src/a4/reader.h src/a4/reader_impl.h src/a4/writer.h src/a4/writer_impl.h
a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py python/a4/protopool.py

protodir=${localstatedir}/a4/proto/$(A4PACK)
protoincludedir=${includedir}/a4/proto/$(A4PACK)
protopythondir=${pythondir}/a4/proto/$(A4PACK)

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

protobuf.make:
	./protobuf.make.sh proto

include protobuf.make

proto_DATA=$(PROTOBUF_PROTO)
protoinclude_HEADERS = $(PROTOBUF_H)
protopython_PYTHON = $(PROTOBUF_PY) $(PYDIR)/__init__.py

lib_LTLIBRARIES=liba4_io.la
liba4_io_la_SOURCES=$(PROTOBUF_CC) src/axis.cpp src/h1.cpp src/h2.cpp src/cutflow.cpp
liba4_io_la_LIBADD=$(protobuf_LIBS)
liba4_io_la_CPPFLAGS=$(protobuf_CFLAGS)

## Test programs
check_PROGRAMS = rwtest test_objstore test_results
rwtest_SOURCES = src/rwtest.cpp
rwtest_CPPFLAGS=$(protobuf_CFLAGS)
rwtest_LDADD=liba4_io.la $(protobuf_LIBS) $(BOOST_SHARED_PTR_LIBS)

test_objstore_SOURCES = src/test_objstore.cpp
test_objstore_LDADD=$(BOOST_SHARED_PTR_LIBS)

test_results_SOURCES = src/test_results.cpp
test_results_CPPFLAGS=$(protobuf_CFLAGS)
test_results_LDADD=liba4_io.la $(protobuf_LIBS) $(BOOST_SHARED_PTR_LIBS)

TESTS=$(check_PROGRAMS)

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho "class [A-Za-z0-9]*" $^ | sed 's/.py:class/ import/' | sed 's/python\/a4\/messages\//from ./' | sed 's/\//./g' > $@


