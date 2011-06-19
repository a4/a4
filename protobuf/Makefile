CCC=g++
CXXFLAGS = ${DEBUG} -fPIC -pipe -Wall -I./ $(PROTOBUF)

PROTOS    = $(wildcard ./proto/*.proto)
PROTOPY   = $(subst ./proto/,./python/,$(patsubst %.proto,%_pb2.py,$(PROTOS)))
PROTOCPP  = $(subst ./proto/,./cpp/,$(patsubst %.proto,%.pb.cc,$(PROTOS)))
PROTOCOBJ = $(subst ./proto/,./obj/,$(patsubst %.proto,%.o,$(PROTOS)))

all: py $(PROTOCPP) $(PROTOCOBJ)

py: $(PROTOPY) python/__init__.py

python/%_pb2.py cpp/%.pb.cc cpp/%.pb.h: proto/%.proto
	mkdir -p python
	mkdir -p cpp
	protoc -I=proto --python_out python --cpp_out cpp $<

python/__init__.py: $(PROTOPY)
	grep -o "class [A-Za-z0-9]*" $(PROTOPY) | sed 's/.py:class/ import/' | sed 's/\.\/python\//from ./' > $@

obj/%.o: cpp/%.pb.cc cpp/%.pb.h	
	mkdir -p obj
	$(CCC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(PROTOCPP)
	rm -f $(PROTOPY)
	rm -f $(PROTOCOBJ)
	rm -f python/__init__.py
