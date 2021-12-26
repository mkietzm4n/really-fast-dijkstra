
typedef struct Edge {
  int to, dist;
} Edge;

typedef struct Vertex {
  int degree;
  Edge* edges;
} Vertex;

typedef struct Graph {
  int order, size;
  Vertex* vertices;
} Graph;

Graph graph_create();
void graph_destroy(Graph graph);