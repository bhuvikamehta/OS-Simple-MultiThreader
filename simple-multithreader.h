#include <iostream>
#include <vector>
#include <functional>
#include <pthread.h>
#include <cstring>

void demonstration(std::function<void()> && lambda) {
    lambda();
}

// Thread argument structure for parallel operations
typedef struct {
    int start, end;
    int low2, high2;
    std::function<void(int)> lambda1;
    std::function<void(int, int)> lambda2;
} ThreadArgs;

// Thread functions
void* parallel_for_1d(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for(int i = args->start; i < args->end; i++) {
        args->lambda1(i);
    }
    return NULL;
}

void* parallel_for_2d(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for(int i = args->start; i < args->end; i++) {
        for(int j = args->low2; j < args->high2; j++) {
            args->lambda2(i, j);
        }
    }
    return NULL;
}

// 1D parallel_for
template<typename Lambda>
void parallel_for(int low, int high, Lambda&& lambda, int numThreads) {
    pthread_t* threads = new pthread_t[numThreads-1];
    ThreadArgs* args = new ThreadArgs[numThreads];
    
    int chunk = (high - low) / numThreads;
    int start = low;
    
    struct timespec begin, end;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    
    // Create threads
    for(int i = 0; i < numThreads-1; i++) {
        args[i].start = start;
        args[i].end = start + chunk;
        args[i].lambda1 = lambda;
        pthread_create(&threads[i], NULL, parallel_for_1d, &args[i]);
        start += chunk;
    }
    
    // Main thread's work
    for(int i = start; i < high; i++) {
        lambda(i);
    }
    
    // Join threads
    for(int i = 0; i < numThreads-1; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    long microseconds = seconds * 1000000 + nanoseconds/1000;
    std::cout << "Execution time: " << microseconds << " microseconds" << std::endl;
    
    delete[] threads;
    delete[] args;
}

// 2D parallel_for
template<typename Lambda>
void parallel_for(int low1, int high1, int low2, int high2, Lambda&& lambda, int numThreads) {
    pthread_t* threads = new pthread_t[numThreads-1];
    ThreadArgs* args = new ThreadArgs[numThreads];
    
    int chunk = (high1 - low1) / numThreads;
    int start = low1;
    
    struct timespec begin, end;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    
    // Create threads
    for(int i = 0; i < numThreads-1; i++) {
        args[i].start = start;
        args[i].end = start + chunk;
        args[i].low2 = low2;
        args[i].high2 = high2;
        args[i].lambda2 = lambda;
        pthread_create(&threads[i], NULL, parallel_for_2d, &args[i]);
        start += chunk;
    }
    
    // Main thread's work
    for(int i = start; i < high1; i++) {
        for(int j = low2; j < high2; j++) {
            lambda(i, j);
        }
    }
    
    // Join threads
    for(int i = 0; i < numThreads-1; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    long microseconds = seconds * 1000000 + nanoseconds/1000;
    std::cout << "Execution time: " << microseconds << " microseconds" << std::endl;
    
    delete[] threads;
    delete[] args;
}

int user_main(int argc, char **argv) {
    int x = 5, y = 1;
    auto lambda1 = [x, &y](void) {
        y = 5;
        std::cout << "====== Welcome to Assignment-" << y << " of the CSE231(A) ======\n";
    };
    demonstration(lambda1);

    // Initialize test data based on program name
    int numThreads = argc > 1 ? atoi(argv[1]) : 4;
    int size = argc > 2 ? atoi(argv[2]) : 100;
    
    if (strstr(argv[0], "vector")) {
        std::vector<int> a(size, 1);
        std::vector<int> b(size, 2);
        std::vector<int> c(size, 0);

        parallel_for(0, size, [&](int i) {
            c[i] = a[i] + b[i];
        }, numThreads);

        bool success = true;
        for(int i = 0; i < size; i++) {
            if(c[i] != 3) {
                success = false;
                break;
            }
        }
        if(success) {
            std::cout << "Test Success\n";
        }
    }
    else if (strstr(argv[0], "matrix")) {
        std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 1));
        std::vector<std::vector<int>> result(size, std::vector<int>(size, 0));

        parallel_for(0, size, 0, size, [&](int i, int j) {
            matrix[i][j] = i + j;
        }, numThreads);

        parallel_for(0, size, 0, size, [&](int i, int j) {
            result[i][j] = matrix[i][j] * 2;
        }, numThreads);

        bool success = true;
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < size; j++) {
                if(result[i][j] != (i + j) * 2) {
                    success = false;
                    break;
                }
            }
            if(!success) break;
        }
        if(success) {
            std::cout << "Test Success.\n";
        }
    }

    auto lambda2 = [](void) {
        std::cout << "====== Hope you enjoyed CSE231(A) ======\n";
    };
    demonstration(lambda2);
    
    return 0;
}

#define main user_main
