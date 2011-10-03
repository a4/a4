A4PACK=a4root

PYDIR=python/a4/proto/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

PROTOBUF_PROTO=$(shell find proto/ -name '*.proto')
PROTOBUF_PY=$(patsubst proto/%.proto,$(PYDIR)/%_pb2.py,$(PROTOBUF_PROTO))
PROTOBUF_CC=$(patsubst proto/%.proto,$(CPPDIR)/%.pb.cc,$(PROTOBUF_PROTO))
PROTOBUF_O=$(patsubst %.cc,%.o,$(PROTOBUF_CC))

SOURCES=$(wildcard src/*.cpp)

CPPFLAGS+=-ggdb $(A4_CPPFLAGS)
CPPFLAGS+=$(shell root-config --cflags) 
CPPFLAGS+=-I$(CPPDIR)
CXX=$(A4_CXX)

LDFLAGS+=$(shell root-config --libs)

.PHONY: all
.SECONDARY: $(PROTOBUF_PY) $(PROTOBUF_CC) $(PROTOBUF_O)

all: analysis rwtest

# Make your binary from the compiled sources and protobuf objects
%: $(PROTOBUF_O) src/%.o
	$(CXX) $(A4_LDFLAGS) $(LDFLAGS) -la4io -la4process -o $@ $^

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(PROTOBUF_PROTO)
	@mkdir -p $(PYDIR) $(CPPDIR)
	${PROTOBUF_PROTOC} -I=proto $(filter -I%, $(A4_CPPFLAGS)) --python_out $(PYDIR) --cpp_out $(CPPDIR) $^

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho "class [A-Za-z0-9]*" $^ | sed 's/.py:class/ import/' | sed 's/python\/a4\/messages\//from ./' | sed 's/\//./g' > $@

clean:
	rm -rf $(PYDIR) $(CPPDIR)
	rm -f $(patsubst %.cpp,%.o,$(SOURCES))


