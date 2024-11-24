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

typedef struct {
    int start;
    int end;
    int low2;
    int high2;
    int numThreads;
    std::function<void(int)> lambda1;
    std::function<void(int, int)> lambda2;
} ThreadArgs;

void* parallel_for_thread1(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->start; i < args->end; ++i) {
        args->lambda1(i);
    }
    free(args);
    pthread_exit(NULL);
}

void* parallel_for_thread2(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->start; i < args->end; ++i) {
        for (int j = args->low2; j < args->high2; ++j) {
            args->lambda2(i, j);
        }
    }
    free(args);
    pthread_exit(NULL);
}

template <typename Lambda>
long long parallel_for_helper(int numThreads, Lambda&& lambda, void* (*thread_func)(void*), ThreadArgs** args_array) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    pthread_t threads[MAX_THREADS];
    
    for (int i = 0; i < numThreads - 1; ++i) {
        pthread_create(&threads[i], NULL, thread_func, args_array[i]);
    }
    
    thread_func(args_array[numThreads - 1]);
    
    for (int i = 0; i < numThreads - 1; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end_time, NULL);
    long long exec_time = (end_time.tv_sec - start_time.tv_sec) * 1000000LL + end_time.tv_usec - start_time.tv_usec;
    printf("Execution time: %lld microseconds\n", exec_time);
    fflush(stdout);  // Force output to be displayed
    
    return exec_time;
}

template <typename Lambda>
long long parallel_for(int low, int high, Lambda&& lambda, int numThreads) {
    // Ensure numThreads is within bounds
    numThreads = std::min(std::max(numThreads, 1), MAX_THREADS);
    
    ThreadArgs** args_array = new ThreadArgs*[numThreads];
    int chunkSize = (high - low + numThreads - 1) / numThreads;
    
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

template <typename Lambda>
long long parallel_for(int low1, int high1, int low2, int high2, Lambda&& lambda, int numThreads) {
    numThreads = std::min(std::max(numThreads, 1), MAX_THREADS);
    
    ThreadArgs** args_array = new ThreadArgs*[numThreads];
    int chunkSize1 = (high1 - low1 + numThreads - 1) / numThreads;
    
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

// Forward declaration of user's main
int user_main(int argc, char** argv);

// Actual main function
int main(int argc, char** argv) {
    setbuf(stdout, NULL);  // Disable output buffering
    
    // Run the user's main function
    int rc = user_main(argc, argv);
    
    // Get number of threads from command line
    int numThreads = argc > 1 ? atoi(argv[1]) : 4;
    
    // Final parallel_for call
    auto finalLambda = [](int i) {};
    long long time1 = parallel_for(0, 100, finalLambda, numThreads);
    printf("Total execution time for parallel_for call 1: %lld microseconds\n", time1);
    fflush(stdout);
    
    return rc;
}

#define main user_main
#endif
