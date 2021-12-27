#include <stdlib.h>
#include "priority_queue.h"

PriorityQueue pq_create(int maxLength, int start) {
  PriorityQueue pq;

  /*
    Skipping 0th index allows me to use division/multiplication by 2 (or in 
    the case of my implementation, bit shift left/right operations) directly 
    when traversing the priority queue, rather than having to +/- 1 first.
  */
  pq.nodes = malloc((maxLength+1)*sizeof(PriorityQueueNode));

  /*
    Add the starting vertex to the queue - simplifies dijkstra initialization
    By not having to repeat code that would otherwise happen in pq_dequeue
  */
  pq.nodes[1] = (PriorityQueueNode) { start, 0 };

  /*
    Length is not an array, just a pointer to int. This is so I can pass the
    priority queue around directly, while still allowing consumers of the 
    priority queue to manipulate the instance's length member. Passing the struct
    directly (i.e. passing a shallow clone of the struct) is better for performance 
    when the contents of the struct are relatively low, because it allows the members 
    to be accessed without dereferencing a ptr.
  */
  pq.length = malloc(sizeof(int));
  *pq.length = 1;

  /*
    This array is multipurpose for improved performance. For a vertex with key i,

      pq.tracker[i] == 0  ==> i has not been discovered yet. I can get away with
        encoding this value as 0 because the index of the priority queue is 1-based.

      pq.tracker[i] == -1 ==> i has been processed.

      pq.tracker[i] > 0   ==> pq.tracker[i] stores the location of that vertex in 
        the priority queue. This is useful for the case when the distance of a 
        node in the queue needs to be updated, as it allows for quick lookup on 
        the key of the vertex (rather than requiring removal followed by re-insertion).
    
    Beyond the algorithmic benefits described in the third condition, there are benefits 
    of encoding all of this state information about the algorithm into a single array. 
    By doing so, I am leveraging the spacial locality principle of the processor's caching 
    algorithm to a greater degree, by allowing the memory retrieved during execution of the 
    algorithm to "cross pollinate" (so to speak).
    
    During execution of the program, memory is retrieved from RAM and cached in a processor in 
    blocks. Arrays are always stored in contiguous sections of memory, making arrays much more 
    performant than similar structures like linked lists. Retrieving one element of an array 
    automatically retrieves and caches the surrounding elements of that array on the same block.
    Retrieving an element of a linked list on the other hand has no such guarantee, since the
    nodes of a linked list only contain references to their neighbor(s) as arbitrary locations in
    memory.

    When the algorithm attempts to determine if a vertex i has been discovered, pq.tracker[i] is
    retrieved from memory and cached. Because tracker is an array, the discovered/undiscovered
    status of the surrounding i's is also cached. Because the array is multipurpose, information 
    about whether the i's have been processed, and their locations in the heap are also cached at
    no extra cost.

    If you were to split this multipurpose array into 2 or 3 distinct arrays, say "discovered", 
    "processed", and "heap_locs", retrieving an element from one array would have no effect on 
    whether the others are cached, and result in ~3X as many L1/L2/L3 cache misses (very costly).
  */
  pq.tracker = calloc(maxLength, sizeof(int));

  return pq;
}

void pq_destroy(PriorityQueue pq) {
  free(pq.nodes);
  free(pq.tracker);
  free(pq.length);
}

void pq_swap(PriorityQueue pq, int a, int b) {
  PriorityQueueNode temp = pq.nodes[a];
  pq.nodes[a] = pq.nodes[b];
  pq.nodes[b] = temp;

  /* Update tracker when performing swap to maintain correct state */
  pq.tracker[pq.nodes[a].id] = a; 
  pq.tracker[temp.id] = b;   
}

void pq_moveUp(PriorityQueue pq, int i) {

  /* Use bit shift operations for improved performance */
  for (int i2=i>>1; i2>0; i=i2, i2=i>>1) {
    if (pq.nodes[i].dist < pq.nodes[i2].dist)
      pq_swap(pq, i, i2);
  }
}

void pq_moveDown(PriorityQueue pq, int i) {
  int hs = *pq.length;

  /* More bit shifting! */
  for (int i2=i<<1, min; i2<=hs; i=min, i2=i<<1) {
    if (i2 + 1 > hs)
      min = i2;
    else if (pq.nodes[i2].dist < pq.nodes[i2+1].dist)
      min = i2;
    else
      min = i2 + 1;
    if (pq.nodes[i].dist > pq.nodes[min].dist)
      pq_swap(pq, i, min);
  }
}

void pq_queue(PriorityQueue pq, int id, int dist) {
  int loc = pq.tracker[id];

  /* if loc == -1, this key has been processed already. */
  if (loc == -1) return; 

  /*
    if loc == 0, this key has not been discovered and
    therefore does not exist in the queue yet. Add it 
    to the queue.
  */
  if (loc == 0) {
    int hs = ++*pq.length;
    pq.nodes[hs].id = id;
    pq.nodes[hs].dist = dist;
    pq.tracker[id] = hs;
    pq_moveUp(pq, hs);
    return;
  }
  
  /*
    else, it exists in the queue already. Check if the
    new distance is less than the previously recorded
    distance. If it is, update. Else, do nothing.
  */
  if (pq.nodes[loc].dist > dist) {
    pq.nodes[loc].dist = dist;
    pq_moveUp(pq, loc);
  }
}

PriorityQueueNode pq_dequeue(PriorityQueue pq) {

  /* Pop minimum node for return */
  PriorityQueueNode node = pq.nodes[1];

  /* Mark this node as processed */
  pq.tracker[node.id] = -1;

  /*
    Move node at the back of the queue to the front,
    then let it trickle back down. This ensures the min
    heap property of the priority queue remains satisfied.
  */
  pq.nodes[1] = pq.nodes[(*pq.length)--];
  pq_moveDown(pq, 1);

  return node;
}