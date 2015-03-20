.PHONY=all clean

CXX=clang++
CXXFLAGS=-std=c++11 -g -Iinclude
LD=clang++
LDFLAGS=
LDLIBS=

SRCS=ops.cpp cpu.cpp main.cpp
OBJS=$(patsubst %.cpp,build/%.o,$(SRCS))
TARGET=nesemul

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(LDLIBS) $(OBJS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf build
	rm -f $(TARGET)

build:
	mkdir build