# OS-Assignment-5-Simple-MultiThreader

Group ID- 58
Group Members:
Bhuvika Mehta (bhuvika23172@iiitd.ac.in)
Pragya Singh (pragya23379@iiitd.ac.in)

Contribution: Both contributed equally and were present at all times. The task was distributed equally within both members.

##Implementations:
This is a C++ header-only file which simplifies parallelizing loops using threads.
Provides two functions for handling parallel workloads;
One for single-dimensional loops and One for nested loops 

execute_time_parallel_for function includes creating threads : pthread_create(), distributing the task for them and waiting for all threads to finish pthread_join() and managing execution time

Demonstration function executes a given lambda function.  

Working:
make
./vector (number of threads) (task size)
./matrix (number of threads) (task size)
