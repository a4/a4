ACLOCAL_AMFLAGS = -I m4

PYDIR=python/a4/messages
CPPDIR=pb

a4includedir=${includedir}/a4
a4pythondir=${pythondir}/a4
msgpythondir=${pythondir}/a4/messages

AM_CPPFLAGS=$(protobuf_CFLAGS)
LIBS=$(protobuf_LIBS)

PROTOBUF_PY=python/a4/messages/A4Stream_pb2.py

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
python/a4/messages/__init__.py: $(PROTOBUF_PY)
	grep -Ho "class [A-Za-z0-9]*" $^ | sed 's/.py:class/ import/' | sed 's/python\/a4\/messages\//from ./' | sed 's/\//./g' > $@

a4include_HEADERS = a4/reader.h a4/writer.h a4/streamable.h pb/A4Stream.pb.h
a4python_PYTHON = python/a4/__init__.py python/a4/A4Stream.py
msgpython_PYTHON = $(PROTOBUF_PY) python/a4/messages/__init__.py

check_PROGRAMS = rwtest
rwtest_SOURCES = pb/A4Stream.pb.cc rwtest.cpp
rwtest_LIBS=$(protobuf_LIBS) $(BOOST_SHARED_PTR_LIBS)

TESTS=rwtest

