ACLOCAL_AMFLAGS = -I m4

PYDIR=python/pb
CPPDIR=src/pb
a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOC} -I=${srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

a4include_HEADERS = src/a4/reader.h src/a4/writer.h src/a4/streamable.h src/pb/A4Stream.pb.h

a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py
