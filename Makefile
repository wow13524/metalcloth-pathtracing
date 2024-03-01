CC := clang++


CFLAGS := -O3 -Wall -std=c++17 -I./shaders -I./include -I./metal-cpp -I./metal-cpp-extensions -fno-objc-arc

LDFLAGS := -framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit -framework MetalPerformanceShaders

OBJECTS := ApplicationDelegate.o Cloth.o Cube.o EventView.mo FloorPlane.o Hdri.o Renderer.o Scene.o SceneObject.o SVGFDenoiser.mo TestScene.o ViewDelegate.o main.o

SHADERS := Denoising.metallib Simulation.metallib Shaders.metallib

all: main

main: $(SHADERS) $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o metalcloth

%.air: shaders/%.metal
	xcrun -sdk macosx metal -o $@ -c $<

%.metallib: %.air
	xcrun -sdk macosx metallib -o $@ $^

%.mo: src/%.mm
	$(CC) $(CFLAGS) -ObjC++ -o $@ -c $<

%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) *.mo *.o *.air *.metallib metalcloth