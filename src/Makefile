CC=gcc
CFLAGS=-I. -lm
DEPS=graph.c graph.h priority_queue.c priority_queue.h timer.c timer.h

all: dijkstra.out

dijkstra.out: $(DEPS) dijkstra.c
	$(CC) -o $@ $^ $(CFLAGS) -O3 -fopenmp

clean:
	\rm -f *.o results*.dat *.out *~ *#
