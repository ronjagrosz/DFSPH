OBJECTS :=\
	main.o\
	Particle.o\
	SPH.o\
	BoundingBox.o\
	Cylinder.o\
	CellList.o\
	Shader.o\
	Viewport.o

SOURCE :=\
	./render/Viewport.cpp\
	./render/Shader.cpp\
	./render/BoundingBox.cpp\
	./render/Cylinder.cpp\
	./particle/CellList.cpp\
	./particle/Particle.cpp\
	./particle/SPH.cpp\
	main.cpp
 
 
 
 
CC := g++
HEADERFILES := -I/usr/local/include/
LDFLAGS := -o
CPFLAGS := -std=c++11 -O3 -c -g


INCLUDES := -I/usr/local/lib/ -framework OpenGL -lglfw -lglew -lfreeimage

all: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) sph $(INCLUDES)

$(OBJECTS): $(SOURCE)
	$(CC) $(HEADERFILES) $(SOURCE) $(CPFLAGS)

clean:
	rm -f *.o
         


