CC = gcc
CFLAGS  = -O3  -ffast-math -fopenmp 
LIBS = -lm -fopenmp
SOURCES = c-ray-mt.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = c-ray-mt

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) -O2 $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) -O2 $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) *.ppm

