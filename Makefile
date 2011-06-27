DEBUG=-O3 -ffast-math
#DEBUG=-g

CXX=g++
CXXFLAGS = ${DEBUG} -pipe -Wall -I./src $(ADD_INCLUDES)
LIBS     = $(ADD_LIBS) -lprotobuf -lboost_filesystem -lboost_system -lboost_thread-mt -lboost_program_options -lpthread
LDFLAGS  = -shared -W1

PYDIR  = ./python/a4/messages
CPPDIR = ./src/pb
OBJDIR = ./obj

PROTOS    = $(wildcard ./messages/*.proto) $(wildcard ./messages/Atlas/*.proto)
SRCS      = $(wildcard ./src/*.cc) 

PROTOPY   = $(subst ./messages/,$(PYDIR)/,$(patsubst %.proto,%_pb2.py,$(PROTOS)))
PROTOCPP  = $(subst ./messages/,$(CPPDIR)/,$(patsubst %.proto,%.pb.cc,$(PROTOS)))
PROTOCOBJ = $(subst ./messages/,$(OBJDIR)/,$(patsubst %.proto,%.o,$(PROTOS)))
PYINIT    = $(PYDIR)/__init__.py

OBJS      = $(foreach obj,$(addprefix ./obj/,$(patsubst %.cc,%.o,$(notdir $(SRCS)))),$(obj))
PROGS     = $(patsubst ./src/%.cpp,./bin/%,$(wildcard ./src/*.cpp))

ROOT_CXXFLAGS = ${DEBUG} -Wall `root-config --cflags` $(CXXFLAGS)
ROOT_LDFLAGS  =`root-config --glibs`
ROOT_SOURCES  = $(wildcard ./root/*.C)
ROOT_PROGS    = $(patsubst ./root/%.cpp,./bin/%,$(wildcard ./root/*.cpp))
ROOT_OBJS     = $(patsubst ./root/%.C,$(OBJDIR)/%.o,$(ROOT_SOURCES))

ALL_SOURCES = $(PROTOCPP) $(SRCS) $(ROOT_SOURCES) $(wildcard ./root/*.cpp) $(wildcard ./src/*.cpp)

all: py $(PROTOCPP) $(PROTOCOBJ) $(OBJS) $(PROGS) $(ROOT_PROGS)

######### DEPENDENCIES
# Add .d to Make's recognized suffixes.
SUFFIXES += .d
#
# #We don't need to clean up when we're making these targets
NODEPS := clean
# #These are the dependency files, which make will clean up after it creates them
DEPFILES := $(patsubst %.C,%.d,$(patsubst %.cc,%.d,$(patsubst %.cpp,%.d,$(ALL_SOURCES))))
#
# #Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
#    #Chances are, these files don't exist.  GMake will create them and
#    #clean up automatically afterwards
    -include $(DEPFILES)
endif

#This is the rule for creating the dependency files
src/%.d: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst src/%.cpp,obj/%.o,$<)' $< > $@

src/%.d: src/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst src/%.cc,obj/%.o,$<)' $< > $@

root/%.d: root/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(ROOT_CXXFLAGS) -MM -MT '$(patsubst root/%.cpp,obj/%.o,$<)' $< > $@

root/%.d: root/%.C
	@mkdir -p $(dir $@)
	$(CXX) $(ROOT_CXXFLAGS) -MM -MT '$(patsubst root/%.C,obj/%.o,$<)' $< > $@
######### END DEPENDENCIES



rootobj: $(ROOT_OBJS)

py: $(PROTOPY) $(PYINIT) $(PYDIR)/Atlas/__init__.py

$(PYDIR)/%_pb2.py $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.h: messages/%.proto
	@mkdir -p $(PYDIR)
	@mkdir -p $(CPPDIR)
	protoc -I=messages --python_out $(PYDIR) --cpp_out $(CPPDIR) $<

$(PYINIT): $(PROTOPY)
	grep -o "class [A-Za-z0-9]*" $(PROTOPY) | sed 's/.py:class/ import/' | sed 's/\.\/python\/a4\/messages\//from ./' | sed 's/\//./g' > $@

$(PYDIR)/Atlas/__init__.py: $(PYINIT)
	@mkdir -p $(dir $@)
	echo > $@

$(OBJDIR)/%.o: $(CPPDIR)/%.pb.cc $(CPPDIR)/%.pb.d $(CPPDIR)/%.pb.h 
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I./src/pb -c $< -o $@

$(OBJDIR)/%.o: ./src/%.cc ./src/%.d $(wildcard ./src/%.h*) $(wildcard ./src/a4/%.h*)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: ./root/%.C ./root/%.d $(wildcard ./root/%.h*) $(wildcard ./root/*/%.h*)
	@mkdir -p $(dir $@)
	$(CXX) $(ROOT_CXXFLAGS) -c $< -o $@

bin/%: src/%.cpp src/%.d $(OBJS) $(PROTOCOBJ) $(wildcard ./src/%.h*)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LIBS) $(filter %.o,$^) $< -o $@

bin/%: root/%.cpp $(ROOT_OBJS) $(OBJS) $(PROTOCOBJ) $(wildcard ./root/%.h*) $(wildcard ./root/*/%.h*)
	@mkdir -p $(dir $@)
	$(CXX) $(ROOT_CXXFLAGS) $(ROOT_LDFLAGS) $(LIBS) $(filter %.o,$^) $< -o $@

pyclean:
	find . -iname "*.pyc" -exec rm {} \;

protoclean:
	rm -f $(PYINIT)
	rm -f $(PROTOCPP)
	rm -f $(PROTOPY)
	rm -f $(PROTOCOBJ)

objclean:
	rm -f $(PROTOCOBJ)
	rm -f $(OBJS)
	rm -f $(ROOT_OBJS)

clean: pyclean objclean protoclean
	find . -iname "*.d" -exec rm {} \;

distclean: clean
	rm $(PYDIR) -rf
	rm $(CPPDIR) -rf


