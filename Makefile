.PHONY=all clean

CXX=g++
CXXFLAGS=-std=c++17 -g
LD=g++
LDFLAGS=
LDLIBS=-lsfml-graphics -lsfml-window -lsfml-system

SRCS=ops.cpp bus.cpp cpu.cpp ppu.cpp sfml_renderer.cpp main.cpp
OBJS=$(patsubst %.cpp,build/%.o,$(SRCS))
TARGET=nesemul

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf build
	rm -f $(TARGET)

build:
	mkdir build
