CC := clang++

CFLAGS := -O3 -Wall -std=c++17 -I./shaders/SharedTypes.h -I./include -I./metal-cpp -I./metal-cpp-extensions -fno-objc-arc

LDFLAGS := -framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit

OBJECTS := ApplicationDelegate.o FloorPlane.o Renderer.o TestScene.o ViewDelegate.o main.o

all: main

main: Shaders.metallib $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o metalcloth

%.air: shaders/%.metal
	xcrun -sdk macosx metal -O3 -o $@ -c $<

Shaders.metallib: Shaders.air
	xcrun -sdk macosx metallib -o $@ $^

%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) *.o *.air *.metallib metalcloth