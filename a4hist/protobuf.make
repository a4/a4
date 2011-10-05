protodir=${localstatedir}/a4/proto/$(A4PACK)
protoincludedir=${includedir}/a4/proto/$(A4PACK)
protopythondir=${pythondir}/a4/proto/$(A4PACK)

PYDIR=./python/a4/proto/$(A4PACK)
CPPDIR=./src/a4/proto/$(A4PACK)
PROTOBUF_CFLAGS += -I$(CPPDIR)

PROTOBUF_PY=$(PYDIR)/Histograms_pb2.py $(PYDIR)/Cutflow_pb2.py
PROTOBUF_H=$(CPPDIR)/Histograms.pb.h $(CPPDIR)/Cutflow.pb.h
PROTOBUF_CC=$(CPPDIR)/Histograms.pb.cc $(CPPDIR)/Cutflow.pb.cc
PROTOBUF_PROTO=$(srcdir)/proto/Histograms.proto $(srcdir)/proto/Cutflow.proto

CLEANFILES += $(PYDIR)/__init__.py

dist_proto_DATA=$(srcdir)/proto/Histograms.proto $(srcdir)/proto/Cutflow.proto
nodist_protoinclude_HEADERS=$(CPPDIR)/Histograms.pb.h $(CPPDIR)/Cutflow.pb.h
nodist_protopython_PYTHON=$(PYDIR)/Histograms_pb2.py $(PYDIR)/Cutflow_pb2.py $(PYDIR)/__init__.py


# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(srcdir)/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOBUF_PROTOC} -I=$(srcdir)/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@

# make sure all protobuf are generated before they are built!
BUILT_SOURCES=$(PROTOBUF_H) $(PROTOBUF_CC)

CLEANFILES += $(PROTOBUF_H) $(PROTOBUF_CC) $(PROTOBUF_PY)


