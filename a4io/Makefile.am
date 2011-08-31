ACLOCAL_AMFLAGS = -I m4

A4PACK=io

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4
a4pythonprotodir=${pythondir}/a4/proto

dist_bin_SCRIPTS=bin/a4dump bin/a4info
dist_pkgdata_SCRIPTS=bin/test_rw
a4include_HEADERS = src/a4/reader.h src/a4/writer.h src/a4/results.h src/a4/streamable.h src/a4/interfaces.h
a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py
a4pythonproto_PYTHON = python/a4/proto/__init__.py

protodir=${localstatedir}/a4/proto/$(A4PACK)
protoincludedir=${includedir}/a4/proto/$(A4PACK)
protopythondir=${pythondir}/a4/proto/$(A4PACK)

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

AM_CPPFLAGS=$(protobuf_CFLAGS)
LDADD=liba4_io.la $(protobuf_LIBS) $(BOOST_SHARED_PTR_LIBS)

include protobuf.make

dist_proto_DATA=$(PROTOBUF_PROTO)
protoinclude_HEADERS = $(PROTOBUF_H)
protopython_PYTHON = $(PROTOBUF_PY) $(PYDIR)/__init__.py

lib_LTLIBRARIES=liba4_io.la
liba4_io_la_SOURCES=$(PROTOBUF_CC) src/a4/objectstore.h src/a4/objectstore_impl.h src/gzip_stream.h \
                    src/gzip_stream.cpp src/reader.cpp src/writer.cpp src/streamable.cpp src/results.cpp
liba4_io_la_LIBADD=$(protobuf_LIBS)
liba4_io_la_CPPFLAGS=$(protobuf_CFLAGS)


## Test programs
check_PROGRAMS = test_rw test_read test_write test_objstore test_results
test_rw_SOURCES = src/test_rw.cpp
test_read_SOURCES = src/test_read.cpp
test_write_SOURCES = src/test_write.cpp
test_objstore_SOURCES = src/test_objstore.cpp
test_objstore_LDADD=$(BOOST_SHARED_PTR_LIBS)
test_results_SOURCES = src/test_results.cpp

TESTS=$(check_PROGRAMS)

installcheck-local: bin/test_rw
	source $(bindir)/this_a4.sh && $(pkgdatadir)/test_rw
	source $(bindir)/this_a4.sh && a4info -m pytest.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump -a pytest.a4 > /dev/null
	./test_read pytest.a4 > /dev/null
	./test_write wtest.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4info -m wtest.a4 > /dev/null
	source $(bindir)/this_a4.sh && a4dump -a wtest.a4 > /dev/null

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@
