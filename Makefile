CC := clang++


CFLAGS := -O3 -Wall -std=c++17 -I./shaders -I./include -I./metal-cpp -I./metal-cpp-extensions -fno-objc-arc

LDFLAGS := -framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit -framework MetalPerformanceShaders

SRC_CPP := $(shell find src -name "*.cpp")
SRC_MM := $(shell find src -name "*.mm")
SRC_OBJECTS := $(SRC_CPP:src/%.cpp=%.o) $(SRC_MM:src/%.mm=%.o)

SRC_METAL := $(shell find shaders -name "*.metal")
SRC_AIR := $(SRC_METAL:shaders/%.metal=%.air)

all: main

main: default.metallib $(SRC_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC_OBJECTS) -o metalcloth

%.air: shaders/%.metal
	xcrun -sdk macosx metal -o $@ -c $<

default.metallib: $(SRC_AIR)
	xcrun -sdk macosx metallib -o $@ $(SRC_AIR)

%.o: src/%.mm
	$(CC) $(CFLAGS) -ObjC++ -o $@ -c $<

%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) *.mo *.o *.air default.metallib metalcloth