SOURCES=$(wildcard src/*.cpp)

CXX=$(shell pkg-config a4 --variable=CXX)
MY_CPPFLAGS+=$(shell pkg-config a4 --cflags)
MY_LDFLAGS=$(shell pkg-config a4 --libs) -la4atlas

.PHONY: all

all: analysis

%: src/%.o
	libtool --mode=link --tag=CC $(CXX) $(MY_LDFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	libtool --mode=compile --tag=CC $(CXX) $(MY_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f $(patsubst %.cpp,%.o,$(SOURCES))

