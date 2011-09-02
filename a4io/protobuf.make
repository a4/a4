PROTOBUF_PY=$(PYDIR)/Cutflow_pb2.py $(PYDIR)/Histograms_pb2.py $(PYDIR)/Results_pb2.py $(PYDIR)/A4Stream_pb2.py
PROTOBUF_H=$(CPPDIR)/Cutflow.pb.h $(CPPDIR)/Histograms.pb.h $(CPPDIR)/Results.pb.h $(CPPDIR)/A4Stream.pb.h
PROTOBUF_CC=$(CPPDIR)/Cutflow.pb.cc $(CPPDIR)/Histograms.pb.cc $(CPPDIR)/Results.pb.cc $(CPPDIR)/A4Stream.pb.cc
PROTOBUF_PROTO=proto/Cutflow.proto proto/Histograms.proto proto/Results.proto proto/A4Stream.proto


# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: ${top_srcdir}/proto/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	${PROTOBUF_PROTOC} -I=${top_srcdir}/proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho 'class [A-Za-z0-9]*' $^ | sed 's/.py:class/ import/' | sed "s/python\/a4\/proto\/$(A4PACK)\//from ./" | sed 's/\//./g' > $@


