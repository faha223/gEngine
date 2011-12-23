# This file makes shit!

#This is the default compiler
CXX = g++
# default compile flags
FLAGS = -Wall -c

# Main build target
all: x86

x86: driver32

x64: driver64

# 32-bit renderer library
libRenderer32.o: renderer.h renderer.cpp libMatrix32.o libVector32.o makefile
	$(CXX) $(FLAGS) -m32 renderer.cpp -o libRenderer32.o

# 64-bit renderer library
libRenderer64.o: renderer.h renderer.cpp libMatrix64.o libVector64.o makefile
	$(CXX) $(FLAGS) -m64 renderer.cpp -o libRenderer64.o

#32-bit mesh library
libMesh32.o: mesh.h mesh.cpp libVector32.o libMatrix32.o makefile
	$(CXX) $(FLAGS) -m32 mesh.cpp -o libMesh32.o

#64-bit mesh library
libMesh64.o: mesh.h mesh.cpp libVector64.o libMatrix64.o makefile
	$(CXX) $(FLAGS) -m64 mesh.cpp -o libMesh64.o

#32-bit SceneGraph
libSceneGraph32.o: sceneGraph.h sceneGraph.cpp renderer.h matrices.h vectorMath.h quaternion.h libMatrix32.o libVector32.o libQuaternion32.o libRenderer32.o
	$(CXX) $(FLAGS) -m32 sceneGraph.cpp -o libSceneGraph32.o

#64-bit SceneGraph
libSceneGraph64.o: sceneGraph.h sceneGraph.cpp renderer.h matrices.h vectorMath.h quaternion.h libMatrix64.o libVector64.o libQuaternion64.o libRenderer64.o
	$(CXX) $(FLAGS) -m64 sceneGraph.cpp -o libSceneGraph64.o

#32-bit driver
driver32: libMesh32.o libRenderer32.o libVector32.o libMatrix32.o libQuaternion32.o libSceneGraph32.o driver.cpp libSteelBattalion32.so
	$(CXX) -Wall -m32 -L/usr/lib32 -L./ libMesh32.o libRenderer32.o libVector32.o libMatrix32.o libQuaternion32.o libSceneGraph32.o driver.cpp -o driver32 -lGL -lGLU -lSDL -lSDL_image -lusb-1.0 -lSteelBattalion32

#64-bit driver
driver64: libMesh64.o libRenderer64.o libVector64.o libMatrix64.o libQuaternion64.o libSceneGraph64.o driver.cpp libSteelBattalion64.so
	$(CXX) -Wall -m64 -L/usr/lib -L./ libMesh64.o libRenderer64.o libVector64.o libMatrix64.o libQuaternion64.o libSceneGraph64.o driver.cpp -o driver64 -lGL -lGLU -lSDL -lSDL_image -lusb-1.0 -lSteelBattalion64

# fake build target for clean-up
clean:
	rm -f libRenderer*
	rm -f libMesh*
	rm -f driver32 driver64
