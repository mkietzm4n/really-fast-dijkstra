CC=gcc
CFLAGS=-I. -lm
DEPS=src/graph.c src/priority_queue.c src/timer.c

all: dijkstra.out

dijkstra.out: $(DEPS) src/dijkstra.c
	$(CC) -o $@ $^ $(CFLAGS) -O3 -fopenmp

clean:
	\rm -f *.o results.txt *.out *~ *#
