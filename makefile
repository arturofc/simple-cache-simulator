COMPILER = gcc
CCFLAGS  = -Wall -ansi -pedantic
all: c-sim
LDFLAGS = -lm

first: c-sim.o
	$(COMPILER) $(CCFLAGS) -o c-sim c-sim.o $(LDFLAGS)
first.o: c-sim.c c-sim.h
	$(COMPILER) $(CCFLAGS) -c c-sim.c 
clean:
	rm -f c-sim c-sim.o
