#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <algorithm>
#include <functional>
#include <iostream>

#define MAX_PARALLEL_THREADS 64 

typedef struct {
    int startIndex;
    int endIndex;
    int startIndex2D;
    int endIndex2D;
    int totalThreads;
    std::function<void(int)> lambda1;
    std::function<void(int, int)> lambda2;
} ThreadArgs; //structure to pass args to threads 

void* execute_paralleltask_2D(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->startIndex; i < args->endIndex; ++i) {
        for (int j = args->startIndex2D; j < args->endIndex2D; ++j) {
            args->lambda2(i, j); //for 2d loops
        }
    }
    pthread_exit(NULL);
}

void* execute_paralleltask_1D(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->startIndex; i < args->endIndex; ++i) {
        args->lambda1(i); //for 1d loops
    }
    pthread_exit(NULL);
}

template <typename Lambda>
long long execution_time_parallel_for(int totalThreads, Lambda&& lambda, void* (*thread_func)(void*), ThreadArgs** args_array) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pthread_t threads[MAX_PARALLEL_THREADS];
    const int worker_threads = totalThreads - 1;  //-1 because of main thread

    for (int i = 0; i < worker_threads; ++i) {
        pthread_create(&threads[i], NULL, thread_func, args_array[i]); //create thread
    }

    if (worker_threads < totalThreads) {  
        ThreadArgs* main_args = args_array[worker_threads];
        if (main_args->lambda2) {  // for 2d loops
            for (int i = main_args->startIndex; i < main_args->endIndex; ++i) {
                for (int j = main_args->startIndex2D; j < main_args->endIndex2D; ++j) {
                    main_args->lambda2(i, j);
                }
            }
        } else {  // for 1d loops
            for (int i = main_args->startIndex; i < main_args->endIndex; ++i) {
                main_args->lambda1(i);
            }
        }
    }

    for (int i = 0; i < worker_threads; ++i) { //waiting for worker threads
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end_time, NULL);
    long long exec_time = (end_time.tv_sec - start_time.tv_sec) * 1000000LL + end_time.tv_usec - start_time.tv_usec;
    printf("Execution time: %lld microseconds\n", exec_time);
    fflush(stdout);

    return exec_time;
}

template <typename Lambda>
long long parallel_for(int low, int high, Lambda&& lambda, int totalThreads) {
    totalThreads = std::min(std::max(totalThreads, 1), MAX_PARALLEL_THREADS);
    ThreadArgs** args_array = new ThreadArgs*[totalThreads];
    int chunkSize = (high - low + totalThreads - 1) / totalThreads;

    for (int i = 0; i < totalThreads; ++i) {
        args_array[i] = static_cast<ThreadArgs*>(malloc(sizeof(ThreadArgs)));
        args_array[i]->startIndex = low + i * chunkSize;
        args_array[i]->endIndex = i == totalThreads - 1 ? high : args_array[i]->startIndex + chunkSize;
        args_array[i]->totalThreads = totalThreads;
        args_array[i]->lambda1 = lambda;
        args_array[i]->lambda2 = nullptr;  // coz lamba2 is null for 1d case
    }

    long long result = execution_time_parallel_for(totalThreads, std::forward<Lambda>(lambda), 
                                         execute_paralleltask_1D, args_array);

    for (int i = 0; i < totalThreads; ++i) {
        free(args_array[i]); 
    }
    delete[] args_array;
    return result;
}

template <typename Lambda>
long long parallel_for(int low1, int high1, int startIndex2D, int endIndex2D, Lambda&& lambda, int totalThreads) {
    totalThreads = std::min(std::max(totalThreads, 1), MAX_PARALLEL_THREADS);
    ThreadArgs** args_array = new ThreadArgs*[totalThreads];
    int chunkSize1 = (high1 - low1 + totalThreads - 1) / totalThreads;

    for (int i = 0; i < totalThreads; ++i) {
        args_array[i] = static_cast<ThreadArgs*>(malloc(sizeof(ThreadArgs)));
        args_array[i]->startIndex = low1 + i * chunkSize1;
        args_array[i]->endIndex = i == totalThreads - 1 ? high1 : args_array[i]->startIndex + chunkSize1;
        args_array[i]->startIndex2D = startIndex2D;
        args_array[i]->endIndex2D = endIndex2D;
        args_array[i]->totalThreads = totalThreads;
        args_array[i]->lambda1 = nullptr;  //coz lambda1 is null for 2d case
        args_array[i]->lambda2 = std::forward<Lambda>(lambda);
    }

    long long result = execution_time_parallel_for(totalThreads, std::forward<Lambda>(lambda), 
                                         execute_paralleltask_2D, args_array);

    for (int i = 0; i < totalThreads; ++i) {
        free(args_array[i]);
    }
    delete[] args_array;
    return result;
}

void demonstration(std::function<void()> &&lambda) {
    lambda();
}

int user_main(int argc, char** argv);


int main(int argc, char** argv) {
    setbuf(stdout, NULL);  

    // demonstration of lambda usage
    int x = 5, y = 1;
    auto lambda1 = [x, &y]() {
        y = 5;
        std::cout << "====== Welcome to Assignment-" << y << " of the CSE231(A) ======\n";
    };
    demonstration(lambda1);

    int rc = user_main(argc, argv);

    auto lambda2 = []() {
        std::cout << "====== Hope you enjoyed CSE231(A) ======\n";
    };
    demonstration(lambda2);

    return rc;
}

#define main user_main
#endif
