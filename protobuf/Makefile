CCC=g++
CXXFLAGS = ${DEBUG} -fPIC -pipe -Wall -I./ $(PROTOBUF)

PROTOS    = $(wildcard ./proto/*.proto)
PROTOPY   = $(subst ./proto/,./python/,$(patsubst %.proto,%_pb2.py,$(PROTOS)))
PROTOCPP  = $(subst ./proto/,./cpp/,$(patsubst %.proto,%.pb.cc,$(PROTOS)))
PROTOCOBJ = $(subst ./proto/,./obj/,$(patsubst %.proto,%.o,$(PROTOS)))

all: $(PROTOPY) $(PROTOCPP) $(PROTOCOBJ)

py: $(PROTOPY)

python/%_pb2.py cpp/%.pb.cc cpp/%.pb.h: proto/%.proto
	mkdir -p python
	mkdir -p cpp
	protoc -I=proto --python_out python --cpp_out cpp $<

obj/%.o: cpp/%.pb.cc cpp/%.pb.h	
	mkdir -p obj
	$(CCC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(PROTOCPP)
	rm -f $(PROTOPY)
	rm -f $(PROTOCOBJ)
