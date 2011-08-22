ACLOCAL_AMFLAGS = -I m4

PYDIR=python/pb
CPPDIR=pb

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

AM_CPPFLAGS = $(protobuf_CFLAGS)
AM_LDFLAGS = $(protobuf_LDFLAGS)

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

a4include_HEADERS = a4/reader.h a4/writer.h a4/streamable.h pb/A4Stream.pb.h
a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py

bin_PROGRAMS = copy
copy_SOURCES = copy.cpp
