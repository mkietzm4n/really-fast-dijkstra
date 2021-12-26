typedef struct PriorityQueueNode {
  int id, dist;
} PriorityQueueNode;

typedef struct PriorityQueue {
  PriorityQueueNode* nodes;
  int* length;
  int* tracker;
} PriorityQueue;

PriorityQueue pq_create(int maxLength, int start);
void pq_destroy(PriorityQueue pq);
void pq_queue(PriorityQueue pq, int id, int dist);
PriorityQueueNode pq_dequeue(PriorityQueue pq);