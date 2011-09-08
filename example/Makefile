A4PACK=mya4

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

PROTOBUF_PROTO=$(shell find proto/ -name '*.proto')
PROTOBUF_PY=$(patsubst proto/%.proto,$(PYDIR)/%_pb2.py,$(PROTOBUF_PROTO))
PROTOBUF_CC=$(patsubst proto/%.proto,$(CPPDIR)/%.pb.cc,$(PROTOBUF_PROTO))

SOURCES=$(wildcard src/*.cpp)

CPPFLAGS+=-I$(CPPDIR)

analyze: $(patsubst %.cpp,%.o,$(SOURCES)) $(patsubst %.cc,%.o,$(PROTOBUF_CC))
	$(CXX) $(LDFLAGS) -la4io -o $@ $^

$(SOURCES): $(PROTOBUF_CC)

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(PROTOBUF_PROTO)
	@mkdir -p $(PYDIR) $(CPPDIR)
	${PROTOBUF_PROTOC} -I=proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $^

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho "class [A-Za-z0-9]*" $^ | sed 's/.py:class/ import/' | sed 's/python\/a4\/messages\//from ./' | sed 's/\//./g' > $@


