OBJECTS :=\
	main.o\
	Particle.o\
	SPH.o\
	Shader.o\
	Viewport.o\
	CellList.o\
	Cylinder.o\
	BoundingBox.o

SOURCE :=\
	./particle/Particle.cpp\
	./render/Viewport.cpp\
	./particle/SPH.cpp\
	./render/Shader.cpp\
	./particle/CellList.cpp\
	./render/Cylinder.cpp\
	./render/BoundingBox.cpp\
	main.cpp
 
 
 
 
CC := g++
HEADERFILES := -I/usr/local/include/ -I/Users/isabelljansson/Documents/DFSPH/ 
LDFLAGS := -o
CPFLAGS := -O3 -c -g


INCLUDES := -framework OpenGL -I/usr/local/lib/ -lglfw3 -lglew -lfreeimage

all: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) sph $(INCLUDES)

$(OBJECTS): $(SOURCE)
	$(CC) $(HEADERFILES) $(SOURCE) $(CPFLAGS)

clean:
	rm -f *.o
         


