#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include <string.h>

FILE* openGraphFile(char* filename, Graph* graph) {
  int buf;
  FILE* file = fopen(filename, "r");
  if (strcmp(filename, "graph2.txt")) {
    
    /* skip headers */
    for (int i = 0; i < 3; i++)
      buf = fscanf(file, "%*[^\n]\n");
  }

  buf = fscanf(file, "%d %d", &(graph->order), &(graph->size));
  return file;
}

void assert(int condition, char* assumption) {
  if (!condition)
    printf("Invalid assumption: %s\n", assumption);
}

void graph_test(char* filename) {
  Graph graph;
  FILE* file = openGraphFile(filename, &graph);
  int buf, from, to, dist, fromPrev, toPrev, max = 0;
  for (int i = 0; i < graph.size; i++) {
    buf = fscanf(file, "%d %d %d", &from, &to, &dist);
    if (i > 0) {
      assert(from >= fromPrev, "from vertices sorted");
      assert(from != fromPrev || to >= toPrev, "'to' vertices sorted");
      assert(from != fromPrev || to != toPrev, "no duplicate edges");
    }
    if (from > max) max = from;
    if (to > max) max = to;
    assert(dist >= 0, "no negative edge weights");
    assert(dist != 0, "no zero edge weights");
    fromPrev = from;
    toPrev = to;
  }
  fclose(file);

  assert(max == graph.order - 1, "Every integer in range included");
}

Graph graph_allocate(char* filename) {
  Graph graph;
  FILE* file = openGraphFile(filename, &graph);
  graph.vertices = calloc(graph.order, sizeof(Vertex));
  for (int i = 0; i < graph.order; i++)
    graph.vertices[i].edges = NULL;

  /*
    Figure out how much memory they need in an initial spin through the data.
    Edges are represented as arrays for enhanced performance - no need to
    dereference a pointer to find the next node in a linked list. This works 
    given the assumptions in graph_test. If graph data is supplied that does 
    not fit these assumptions, an error message will be printed to stdout.
  */
  int fromPrev, buf;
  buf = fscanf(file, "%d %*d %*d", &fromPrev);
  for (int i = 1, t = 0, from; i <= graph.size; i++) {
    buf = fscanf(file, "%d %*d %*d", &from);
    if (from != fromPrev | i == graph.size) {
      int degree = i - t;
      graph.vertices[fromPrev].degree = degree;
      graph.vertices[fromPrev].edges = malloc(degree*sizeof(Edge));
      fromPrev = from;
      t += degree;
    }
  }
  fclose(file);
  return graph;
}

void graph_populate(Graph graph, char* filename) {
  FILE* file = openGraphFile(filename, &graph);

  /* Memory has been allocated for edges, now populate them  */
  Edge edge;
  int buf;
  for (int i = 0, j = 0, from; i < graph.size; i++) {
    buf = fscanf(file, "%d %d %d", &from, &edge.to, &edge.dist);
    graph.vertices[from].edges[j] = edge;
    if (graph.vertices[from].degree == ++j) j = 0;
  }
  fclose(file);
}

Graph graph_create(char* filename) {
  graph_test(filename);
  Graph graph = graph_allocate(filename);
  graph_populate(graph, filename);
  return graph;
}

void graph_destroy(Graph graph) {
  for (int i = 0; i < graph.order; i++) {
    if (graph.vertices[i].degree != 0)
      free(graph.vertices[i].edges);
  }
  free(graph.vertices);
}
