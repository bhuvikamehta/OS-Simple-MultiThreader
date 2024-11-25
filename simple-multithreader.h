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

#define MAX_THREADS 64

typedef struct {
    int start;
    int end;
    int low2;
    int high2;
    int numThreads;
    std::function<void(int)> lambda1;
    std::function<void(int, int)> lambda2;
} ThreadArgs; //data required by the threads 

void* parallel_for_thread1(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->start; i < args->end; ++i) {
        args->lambda1(i); //for single dimensional loops 
    }
    pthread_exit(NULL);
}

void* parallel_for_thread2(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    for (int i = args->start; i < args->end; ++i) {
        for (int j = args->low2; j < args->high2; ++j) {
            args->lambda2(i, j); //for nested loops
        }
    }
    pthread_exit(NULL);
}

template <typename Lambda>
long long parallel_for_helper(int numThreads, Lambda&& lambda, void* (*thread_func)(void*), ThreadArgs** args_array) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pthread_t threads[MAX_THREADS];
    const int worker_threads = numThreads - 1;  

    for (int i = 0; i < worker_threads; ++i) {
        pthread_create(&threads[i], NULL, thread_func, args_array[i]); //thread creations
    }
 
    if (worker_threads < numThreads) {  
        ThreadArgs* main_args = args_array[worker_threads];
        if (main_args->lambda2) {  
            for (int i = main_args->start; i < main_args->end; ++i) { //nested loops
                for (int j = main_args->low2; j < main_args->high2; ++j) {
                    main_args->lambda2(i, j);
                }
            }
        } else { 
            for (int i = main_args->start; i < main_args->end; ++i) { //signle dimenstional 
                main_args->lambda1(i);
            }
        }
    }

    for (int i = 0; i < worker_threads; ++i) { //waiting for threads
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end_time, NULL);
    long long exec_time = (end_time.tv_sec - start_time.tv_sec) * 1000000LL + end_time.tv_usec - start_time.tv_usec;
    printf("Execution time: %lld microseconds\n", exec_time);
    fflush(stdout);

    return exec_time;
}

template <typename Lambda>
long long parallel_for(int low, int high, Lambda&& lambda, int numThreads) {
    numThreads = std::min(std::max(numThreads, 1), MAX_THREADS);
    ThreadArgs** args_array = new ThreadArgs*[numThreads];
    int chunkSize = (high - low + numThreads - 1) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        args_array[i] = static_cast<ThreadArgs*>(malloc(sizeof(ThreadArgs)));
        args_array[i]->start = low + i * chunkSize;
        args_array[i]->end = i == numThreads - 1 ? high : args_array[i]->start + chunkSize;
        args_array[i]->numThreads = numThreads;
        args_array[i]->lambda1 = lambda;
        args_array[i]->lambda2 = nullptr;  // lambda2 is null for single dimenstional case
    }

    long long result = parallel_for_helper(numThreads, std::forward<Lambda>(lambda), 
                                         parallel_for_thread1, args_array);

    for (int i = 0; i < numThreads; ++i) {
        free(args_array[i]); //memory cleanup 
    }
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
        args_array[i]->lambda1 = nullptr;  //  lambda1 is null for 2d
        args_array[i]->lambda2 = std::forward<Lambda>(lambda);
    }

    long long result = parallel_for_helper(numThreads, std::forward<Lambda>(lambda), 
                                         parallel_for_thread2, args_array);

    for (int i = 0; i < numThreads; ++i) {
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
