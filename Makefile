CCC=g++
CXXFLAGS = ${DEBUG} -fPIC -pipe -Wall -I./ $(ADD_INCLUDES)
LIBS     = $(ADD_LIBS) -lprotobuf -lboost_filesystem -lboost_system -lboost_thread-mt -lboost_program_options -lpthread
LDFLAGS  = -shared -W1

PYDIR  = ./python/a4/messages
CPPDIR = ./src/pb
OBJDIR = ./obj

PROTOS    = $(wildcard ./messages/*.proto)
SRCS      = $(wildcard ./src/*.cc) 

PROTOPY   = $(subst ./messages/,$(PYDIR)/,$(patsubst %.proto,%_pb2.py,$(PROTOS)))
PROTOCPP  = $(subst ./messages/,$(CPPDIR)/,$(patsubst %.proto,%.pb.cc,$(PROTOS)))
PROTOCOBJ = $(subst ./messages/,$(OBJDIR)/,$(patsubst %.proto,%.o,$(PROTOS)))
PYINIT    = $(PYDIR)/__init__.py

OBJS      = $(foreach obj,$(addprefix ./obj/,$(patsubst %.cc,%.o,$(notdir $(SRCS)))),$(obj))
PROGS     = $(patsubst ./src/%.cpp,./bin/%,$(wildcard ./src/*.cpp))


ROOT_CXXFLAGS =-c -g -Wall `root-config --cflags` $(CXXFLAGS)
ROOT_LDFLAGS  =`root-config --glibs`
ROOT_SOURCES  = $(wildcard ./root/*.C)
ROOT_PROGS    = $(patsubst ./root/%.cpp,./bin/%,$(wildcard ./root/*.cpp))
ROOT_OBJS     = $(patsubst ./root/%.C,$(OBJDIR)/%.o,$(ROOT_SOURCES))



all: py $(PROTOCPP) $(PROTOCOBJ) $(OBJS) $(PROGS) $(ROOT_PROGS)

rootobj: $(ROOT_OBJS)

py: $(PROTOPY) $(PYINIT)

$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: messages/%.proto
	mkdir -p $(PYDIR)
	mkdir -p $(CPPDIR)
	protoc -I=messages --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

$(PYINIT): $(PROTOPY)
	grep -o "class [A-Za-z0-9]*" $(PROTOPY) | sed 's/.py:class/ import/' | sed 's/\.\/python\/a4\/messages\//from ./' > $@

$(OBJDIR)/%.o: $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h	
	mkdir -p $(OBJDIR)
	$(CCC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: ./src/%.cc
	$(CCC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: ./root/%.C
	$(CCC) $(ROOT_CXXFLAGS) -c $< -o $@

bin/%: src/%.cpp $(OBJS) $(PROTOCOBJ)
	$(CCC) $(CXXFLAGS) $(LIBS) $^ -o $@

bin/%: root/%.cpp $(ROOT_OBJS)
	$(CCC) $(ROOT_CXXFLAGS) $< -o $(OBJDIR)/$*.o
	$(CCC) $(ROOT_LDFLAGS) $(LIBS) $(ROOT_OBJS) $(OBJDIR)/$*.o -o $@

clean:
	rm -f $(PROTOCPP)
	rm -f $(PROTOPY)
	rm -f $(PROTOCOBJ)
	rm -f $(PYINIT)
	rm -f $(OBJS)
