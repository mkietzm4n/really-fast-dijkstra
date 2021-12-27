typedef struct PriorityQueueNode {
  /*
    Storing dist on the node itself (as opposed to a distinct "dist"
    array, indexed on id) results in improved performance
    for similar reasons to those outlined on lines 32-66 of 
    priority_queue.c. When two pieces of data are frequently 
    (if not always) used in conjunction, it is more efficient to 
    store those pieces in neighboring locations of memory. My observation
    was, whenever I access a node from the queue, I end up needing
    the associated distance too. So why not store them together, such
    that retrieving and caching one will automatically retrieve and
    cache the other?
  */
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