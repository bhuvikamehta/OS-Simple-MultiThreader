#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <algorithm>
#include <functional>

#define MAX_THREADS 64

// Structure to hold arguments for thread functions
typedef struct {
    int start;
    int end;
    int low2;
    int high2;
    int numThreads;
    std::function<void(int)> lambda1;
    std::function<void(int, int)> lambda2;
} ThreadArgs;

// Function for parallel execution of lambda1 using multiple threads
void* parallel_for_thread1(void* arg) {
    // Cast the generic argument to the specific ThreadArgs structure
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    
    // Loop over the specified range and execute lambda1 for each iteration
    for (int i = args->start; i < args->end; ++i) {
        args->lambda1(i);
    }
    free(args);
    pthread_exit(NULL);
}

// Function for parallel execution of lambda2 using multiple threads
void* parallel_for_thread2(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    
    // Nested loop over the specified ranges and execute lambda2 for each iteration
    for (int i = args->start; i < args->end; ++i) {
        for (int j = args->low2; j < args->high2; ++j) {
            args->lambda2(i, j);
        }
    }
    free(args);
    pthread_exit(NULL);
}

// Helper function for thread creation and joining
template <typename Lambda>
long long parallel_for_helper(int numThreads, Lambda&& lambda, void* (*thread_func)(void*), ThreadArgs** args_array) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    pthread_t threads[MAX_THREADS];
    
    // Create numThreads-1 worker threads
    for (int i = 0; i < numThreads - 1; ++i) {
        pthread_create(&threads[i], NULL, thread_func, args_array[i]);
    }
    
    // Execute lambda in main thread using the last set of arguments
    thread_func(args_array[numThreads - 1]);
    
    // Wait for worker threads
    for (int i = 0; i < numThreads - 1; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end_time, NULL);
    long long exec_time = (end_time.tv_sec - start_time.tv_sec) * 1000000LL + end_time.tv_usec - start_time.tv_usec;
    printf("Execution time: %lld microseconds\n", exec_time);
    
    return exec_time;
}

// 1D version
template <typename Lambda>
long long parallel_for(int low, int high, Lambda&& lambda, int numThreads) {
    ThreadArgs** args_array = new ThreadArgs*[numThreads];
    int chunkSize = (high - low + numThreads - 1) / numThreads;
    
    // Prepare arguments for all threads (including main thread)
    for (int i = 0; i < numThreads; ++i) {
        args_array[i] = static_cast<ThreadArgs*>(malloc(sizeof(ThreadArgs)));
        args_array[i]->start = low + i * chunkSize;
        args_array[i]->end = i == numThreads - 1 ? high : args_array[i]->start + chunkSize;
        args_array[i]->numThreads = numThreads;
        args_array[i]->lambda1 = lambda;
    }
    
    long long result = parallel_for_helper(numThreads, std::forward<Lambda>(lambda), 
                                         parallel_for_thread1, args_array);
    
    delete[] args_array;
    return result;
}

// 2D version
template <typename Lambda>
long long parallel_for(int low1, int high1, int low2, int high2, Lambda&& lambda, int numThreads) {
    ThreadArgs** args_array = new ThreadArgs*[numThreads];
    int chunkSize1 = (high1 - low1 + numThreads - 1) / numThreads;
    
    // Prepare arguments for all threads (including main thread)
    for (int i = 0; i < numThreads; ++i) {
        args_array[i] = static_cast<ThreadArgs*>(malloc(sizeof(ThreadArgs)));
        args_array[i]->start = low1 + i * chunkSize1;
        args_array[i]->end = i == numThreads - 1 ? high1 : args_array[i]->start + chunkSize1;
        args_array[i]->low2 = low2;
        args_array[i]->high2 = high2;
        args_array[i]->numThreads = numThreads;
        args_array[i]->lambda2 = std::forward<Lambda>(lambda);
    }
    
    long long result = parallel_for_helper(numThreads, std::forward<Lambda>(lambda), 
                                         parallel_for_thread2, args_array);
    
    delete[] args_array;
    return result;
}
// User-defined main function
int user_main(int argc, char** argv);

// Main function 
int main(int argc, char** argv) {
    int numThreads = 4;  
    int rc = user_main(argc, argv);

    auto lambda2 = [](int) {
        //printf("====== Hope you enjoyed CSE231(A) ======\n");
    };

    long long time1 = parallel_for(0, 100, lambda2, numThreads);
    //printing execution time
    printf("Total execution time for parallel_for call 1: %lld microseconds\n", time1);

    return rc;
}

#define main user_main
#endif
