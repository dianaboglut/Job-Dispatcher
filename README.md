# Server-Cluster-Simulation-with-MPI
This project simulates a server cluster architecture where the Main Server acts as a job dispatcher. It continuously receives client commands and dispatches them to available Worker Servers within the cluster. If there are no available worker servers, the main server waits until a worker finishes its current task before dispatching the next job. Worker servers process commands and send results back to the main server, which stores the results in files for each client and logs the status of each task.

The commands the server cluster handles are:

1. PRIMES N: Find out how many prime numbers exist in the first N natural numbers.
2. PRIMEDIVISORS N: Determine how many prime divisors the number N has.
3. ANAGRAMS name: Generate all anagrams (permutations) of the given name (up to 8 characters).

The simulation is managed using MPI (Message Passing Interface), ensuring proper synchronization between the main server and worker servers, whether the processes are running on a single machine or distributed across multiple machines.

### Command File Example
```
CLI0 PRIMEDIVISORS 452876
WAIT 2
CLI1 PRIMES 1000
CLI2 ANAGRAMS "example"
```
## Architecture
- Main Server: Handles client requests, dispatches tasks to worker servers, manages job queue, and logs results.
- Worker Servers: Process commands and send results back to the main server.
- MPI Communication: MPI ensures the synchronization between servers. Worker servers receive tasks, process them, and send results back to the main server.

## Compile and Run the program
### Compiling with/without debugging
```
make
make debug
```
### Running with/without debugging
```
make run
```
```
make run_debug
```
