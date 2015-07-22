.PHONY=all clean

CXX=clang++
CXXFLAGS=-std=c++11 -g -Iinclude `sdl2-config --cflags`
LD=clang++
LDFLAGS=
LDLIBS=`sdl2-config --libs`

SRCS=ops.cpp cpu.cpp ppu.cpp sdl_renderer.cpp main.cpp
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
