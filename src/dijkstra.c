#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "graph.h"
#include "priority_queue.h"
#include "timer.h"

typedef struct Result {
  int start;
  PriorityQueueNode node;
} Result;

int cmpfunc (const void * a, const void * b) {
  return ((Result *)b)->node.dist - ((Result *)a)->node.dist;
}

void dijkstra(Graph graph, int start, Result* results) {
  PriorityQueue pq = pq_create(graph.order, start);

  /*
    Processed nodes are guaranteed to be removed from the priority queue
    in order of least to greatest dist. Therefore, the items last removed
    from the queue with have the greatest dist. I accumulate items removed 
    from the queue in this rank array, then collect 10 nodes (or as many as
    were processed, whichever is fewer) from the back of rank after dijkstra 
    is complete. This improves performance by avoiding sorting of the entire
    list of distance results after dijkstra is complete.
  */
  PriorityQueueNode* rank = malloc(graph.order*sizeof(PriorityQueueNode));

  /*
    Declare i outside of for loop - after dijkstra is complete, i represents
    the number of vertices that were processed.
  */
  int i;
  for (i = 0; *pq.length > 0; i++) {

    /*
      pq starts out populated with the starting node, so this will succeed
      on the first iteration
    */
    rank[i] = pq_dequeue(pq);

    /* rank[i] is the current node being processed */
    Vertex v = graph.vertices[rank[i].id];

    /* Attempt to queue all adjacent vertices */
    for (int j = 0; j < v.degree; j++)
      pq_queue(pq, v.edges[j].to, rank[i].dist + v.edges[j].dist);
  }

  /* Add nodes with greatest dist to the designated section of the results array */
  for (int k = 0; k < (i < 10 ? i : 10); k++)
    results[start*10 + k].node = rank[i - 1 - k];

  pq_destroy(pq);
  free(rank);
}

int main(const int argc, char** argv) {
  if (argc != 2) {
    printf("Expected 1 argument aside from executable name (filename), received %d\n", argc - 1);
    return -1;
  }
  char* filename = argv[1];
  Graph graph = graph_create(filename);
  
  /*
    Each vertex is allotted a range of 10 indices to store the top
    ten paths from execution of dijkstra. Note that this results
    array is guaranteed to contain the global top ten paths once
    dijkstra has been completed for every vertex.
  */
  Result* results = malloc(10*graph.order*sizeof(Result));
  for (int i = 0; i < 10*graph.order; i++) {
    results[i].start = i / 10;
    results[i].node.dist = -1;
  }
  
  /* See "performance" section of README.md for more detail on this part */
  StartTimer();
  #pragma omp parallel for schedule(dynamic, 4)
  for (int i = 0; i < graph.order; i++)
    dijkstra(graph, i, results);
  const double time = GetTimer() / 1000.0;

  /* Output results to results.txt */
  FILE* file = fopen("results.txt", "w"); 
  fprintf(file, "Graph: %s\n", filename);
  fprintf(file, "Duration: %lf (s)\n\n", time);
  qsort(results, 10*graph.order, sizeof(Result), cmpfunc);
  for (int i = 0; i < 10; i++)
    fprintf(file, "%d %d %d\n", results[i].start, results[i].node.id, results[i].node.dist);
  fclose(file);

  free(results);
  graph_destroy(graph);
  return 0;
}