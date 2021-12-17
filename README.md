# Really Fast Dijkstra
# Intro
This code is really fast. It's so fast, it'll make you wonder "Dang, how'd he make that code so dang fast?". Here's a quick comparison to show just how fast this really fast code is:

(Execution durations for graph2.txt processing)
- Best code from last year's class with 112 threads: 125.48(s)
  - 56 core machine
- My code with 144 threads: 19.39(s)
  - 18 cores (JSLE Cooper Lake node, details in Hardware.txt)
- My code with 32 threads: 75.34(s)
  - 16 cores (uchicago linux6)
- My code with 16 threads: 128.62(s)
  - 16 cores (uchicago linux6)

~6.5X faster! :rocket::rocket:
Celebrity endorsements:

<p align="center">
"wow, that is really fast." - Kyle Felker, 2021
</p>
<p align="center">
":rocket:" - Amanda Lund, 2021
</p>
<p align="center">
"Vroom vroom." - Edsger Dijkstra, 1959
</p>

# Optimizations
Here are the optimizations I made beyond the original project specifications ordered by impact on performance, with 1 having the greatest impact, and 7 having a trivial impact. I have not collected any data to support these rankings, they are simply based on my understanding of algorithms and system memory management. Further details/explanations for each item are available in the associated comments.
- (1) Added a multipurpose "tracker" array member to the priority queue struct. See comments on lines 32-38 of priority_queue.c
- (2) Stored distance associated with each vertex on the PriorityQueueNode struct itself, rather than maintaining a seperate "dist" array as was used in the template dijkstra implementation. See comments on lines 3-7 of prioirity_queue.h
- (3) Collected longest path results during disassembly of the priority queue. See comments on lines 21-27 of dijkstra.c
- (4) Stored the outgoing edges of each vertex as an array (as opposed to a linked list). See comments on lines 56-60 of graph.c. This one seemed like a powerful optimization to me - I put it lower on the list since I saw some others in the class had the same idea, but got different performance results
- (5) Passed prioirity queue to pq methods directly (no pointer). See comments on lines 21-26 of prioirity_queue.c
- (6) Used bit shifting in place of division/multiplication by 2 while navigating the priority queue
- (7) Implemented a 1-based index priority queue (really just makes code cleaner). See comments on lines 8-10 of priority_queue.c

Reference used for implementation of prioirity queue using a min binary heap: https://bradfieldcs.com/algos/trees/priority-queues-with-binary-heaps/

# Instructions
To compile this project in a Unix or Linux distribution, first clone my repository to your machine and ensure you have gcc installed. 
Then open a terminal instance and navigate to your clone of my repository. Once inside the repository root folder, enter the following command:
```bash
$ make
```
This command will output one executable: dijkstra.out. To reproduce my results for graph1 or graph2, supply the corresponding filename as a command line argument:
```bash
$ ./dijkstra.out [filename]
```
Notes: 
- I did not add any validation on the name of the provided file - if you run the program and get a segmentation fault, check you have the filename spelled right (or in the case of graph2, that you've unzipped `graph2.txt.gz` to `graph2.txt`).
- Results are printed to a file `results.txt`, and are not printed to stdout.

# Results
Obtained top 10 longest path results for graph1 and graph2, ordered by distance desc, start asc, then end asc.

### Graph1
Start Vertex | End Vertex | Distance
---          | ---        | ---
644          | 6278       | 192
644          | 6290       | 186
5720         | 6278       | 186
645          | 6278       | 184
3634         | 6278       | 184
644          | 6187       | 183
4568         | 6278       | 183
644          | 6188       | 181
4193         | 6278       | 181
5995         | 6278       | 181

### Graph2
Start Vertex | End Vertex | Distance
---          | ---        | ---
100201       | 36397      | 103
100201       | 150246     | 103
41354        | 36397      | 102
41354        | 150246     | 102
100201       | 139117     | 102
100201       | 92762      | 102
26333        | 36397      | 101
26333        | 150246     | 101
40743        | 36397      | 101
40743        | 150246     | 101

# Performance
For each graph, recorded execution duration is the mean of 5 trials, with the exception of the serial case for Graph2, of which I did 3 trials (I include only the averages for brevity). All executions were run on the uchicago linux cluster (`linux6`, `nproc` yields 64).

### Graph1
Parallel (Yes/No)  | Execution Duration (s) | Speedup
---                | ---                      | ---
No                 | 1.765                    | --
Yes (omp, static)  | 0.097                    | 18.196
Yes (omp, dynamic) | 0.068                    | 25.956

### Graph2
Parallel (Yes/No)  | Execution Duration (s) | Speedup
---                | ---                      | ---
No                 | 1819.077                 | --
Yes (omp, static)  | 74.483                   | 24.422
Yes (omp, dynamic) | 54.154                   | 33.591

Trials of the serial case were performed with the OpenMP pragma commented out. Trials using OpenMP used the following OpenMP code:
```
#pragma omp parallel for //static scheduling
```
```
#pragma omp parallel for schedule(dynamic, 4) // dynamic scheduling
```
- Note that in neither case is the number of threads explicitly set - this gives control to omp to determine the optimal number of threads of execution based on the resources of the underlying system, allowing for greater speedup on machines with a larger number of cores than if `num_threads(16)` was used, for example. 
- `schedule(dynamic, 4)` works to override the default behavior of statically partitioning the index space of the for loop among the available threads. `schedule(dynamic, 4)` allows regions of work on the index space to be assigned, 4 indices at a time, to threads as they become available. This scheduling strategy is particularly beneficial in this algorithm (large increase in speedup between static vs dynamic), because completing dijkstra for some vertices may be trivial (e.g. vertex has no outgoing edges, or is part of a small, strongly connected component that has no outgoing edges to the rest of the graph), while dijkstra may be very involved for other vertices (e.g. every vertex in the graph can be reached starting at such a vertex);
- My speedup results align with my understanding of strong vs weak scaling - For both static and dynamic omp scheduling, graph2 experienced greater speedup than graph1.
- It is expected that speedup << # of cores. A contributing factor is that the 64 cores advertised by `nproc` are not physical cores but logical cores - there are 16 physical cores on linux6, each core being capable of handling 4 threads. Although, even if all cores were physical cores, speedup would still be less than # of cores, due to overhead from thread management, the cache coherency protocol of the processor, etc.

# Valgrind
When testing dijkstra.out against custom_graph.txt, graph1.txt, and graph2.txt, valgrind consistently gave me a report of the following:
* possibly lost: 6624 bytes in 23 blocks
* still reachable:	6928 bytes in 4 blocks

These errors disappeared when I commented out my usage of OpenMP, letting the program run in serial. Concerned that these issues might be a result of the parallelized code, I uncommented my usage of OpenMP and used num_threads (1) to force the program to run in serial while still using OpenMP. The valgrind errors persisted even in this case. Since the valgrind errors...
1. Persisted in both serial and parallel usages of OpenMP
2. Disappeared in serial without OpenMP, giving a clean report
3. Were the same for all three graph datafiles (and did not scale with the size of the graph)

I can only assume it is the usage of OpenMP that is causing the memory leaks (or simply throwing off valgrind), and not my code.

# Testing
- To test my implementation of dijkstra and the priority queue using a binary heap, I created a graph of order 10 and size 16. I made the graph not strongly connected for better testing of edge cases - vertices 0 and 5 have no outgoing edges, and vertex 6 only has outgoing edges. To ensure correctness of my code, I manually performed dijkstra's algorithm for all 10 vertices and collected the 10 longest paths, comparing them with the results of my program. I found the results to be identical, giving me confidence that the above results section is accurate. You can see the results of my manual dijkstra calculations in custom_graph.xlsx. 
- I also made my graph_create function perform tests to ensure the integrity of the graph datafile provided, printing to stdout if any assumptions I have made about the data provided are invalid. It is expected for these tests to be quiet and not produce any output during the regular execution of dijkstra.out. The tests have ~0 affect on the execution duration of the program.