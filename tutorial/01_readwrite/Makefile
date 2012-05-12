A4PACK=tutorial

PYDIR=python/$(A4PACK)
CPPDIR=src/a4/proto/$(A4PACK)

PROTOBUF_PROTO=$(shell find proto/ -name '*.proto')
PROTOBUF_PY=$(patsubst proto/%.proto,$(PYDIR)/%_pb2.py,$(PROTOBUF_PROTO))
PROTOBUF_CC=$(patsubst proto/%.proto,$(CPPDIR)/%.pb.cc,$(PROTOBUF_PROTO))
PROTOBUF_O=$(patsubst %.cc,%.o,$(PROTOBUF_CC))
PROTOBUF_PYPACK=$(PROTOBUF_PY) $(foreach protopy,$(dir $(PROTOBUF_PY)),$(protopy)__init__.py)

SOURCES=$(wildcard src/*.cpp)

CXX=$(shell pkg-config a4 --variable=CXX)
PROTOC=$(shell pkg-config a4 --variable=PROTOC)
MY_CPPFLAGS+=$(shell pkg-config a4 --cflags) -I$(CPPDIR)
MY_LDFLAGS=$(shell pkg-config a4 --libs)

.PHONY: all
.SECONDARY: $(PROTOBUF_PY) $(PROTOBUF_CC) $(PROTOBUF_O)

all: analysis rwtest $(PROTOBUF_PYPACK)

# Make your binary from the compiled sources and protobuf objects
%: $(PROTOBUF_O) src/%.o
	libtool --mode=link --tag=CC $(CXX) $(MY_LDFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	libtool --mode=compile --tag=CC $(CXX) $(MY_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

%.o: %.cc
	libtool --mode=compile --tag=CC $(CXX) $(MY_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

# how to make protobuf objects
$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: $(PROTOBUF_PROTO)
	@mkdir -p $(PYDIR) $(CPPDIR)
	$(PROTOC) -I=proto --python_out $(PYDIR) --cpp_out $(CPPDIR) $^

# how to make the python __init__.py
$(PYDIR)/__init__.py: $(PROTOBUF_PY)
	grep -Ho "class [A-Za-z0-9]*" $^ | sed 's/.py:class/ import/' | sed 's/$(subst /,\/,$(PYDIR))\//from ./' | sed 's/\//./g' > $@

$(PYDIR)/%/__init__.py:
	touch $@

clean:
	rm -rf $(PYDIR) $(CPPDIR)
	rm -f $(patsubst %.cpp,%.o,$(SOURCES))

