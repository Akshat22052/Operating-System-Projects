#include <chrono>
#include <iostream>
#include<list>
#include <functional>
#include <stdlib.h>
#include <cstring>

long long t_time = 0;
int thread_cnt = 0;
int user_main(int argc, char **argv);
void demonstration(std::function<void()> && lambda) {
    lambda();
}
// structure for matrix problem;
typedef struct {
    int low1;
    int high1;
    int low2;
    int high2;
    int size;
    std::function<void(int, int)> lambda;
}thread_args1;

//thread function for matrix problem
void* thread_func1(void *ptr)
{
    thread_args1* t = (thread_args1 *) (ptr);
    for (int i= t->low1; i<t->high1; i++) {
        for (int j=t->low2; j<t->high2; j++){
            t->lambda(i,j);
        }
    }
    return NULL;
}

// parallel for matrix problem
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads)
{
    thread_cnt++;
    auto started = std::chrono::high_resolution_clock::now();
    pthread_t tid[numThreads];
    thread_args1 args[numThreads];
    if (high1 - low1  <= 10) {
        numThreads = 1;
    }
    int chunk = (high1-low1)/numThreads;
    // setting the thread ids and and their arguements
    for (int i=0; i<numThreads; i++)
    {
        args[i].low1 = i*chunk;
        // handling case if size is not multiple of threads
        if(i==numThreads-1) {
            args[i].high1 = high1;
        }
        else {
            args[i].high1 = (i+1)*chunk;
        }
        args[i].high2 = high2;
        args[i].low2 = low2;
        args[i].size = high1 - low1;
        args[i].lambda = lambda;
        if (pthread_create(&tid[i], NULL, thread_func1, (void*)(&args[i])) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }

    }
   
    // joining the threads
    for (int i=0; i<numThreads; i++)
    {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
        }
    }
    
    // printing the time
    auto done = std::chrono::high_resolution_clock::now();
    std :: cout << "\ntime in milli seconds for thread " <<thread_cnt <<": "  ;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count();
    std::cout << "millis seconds\n";
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();
    t_time+=duration;
}

// struct for vector problem
typedef struct {
    int low;
    int high;
    std::function<void(int)> lambda;
}thread_args;

// thread function for the vector problem
void* thread_func(void *ptr)
{
    thread_args* t = (thread_args *) (ptr);
    for (int i= t->low; i<t->high; i++)
    {
        t->lambda(i);
    }
    return NULL;

}

// threading function to create an run the threads for vector problem
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads)
{
    thread_cnt++;
    auto started = std::chrono::high_resolution_clock::now();
    
    // array for storing the arguements and thread ids for various threads
    pthread_t tid[numThreads];
    
    thread_args args[numThreads];
    if (high - low  <= 100) {
        numThreads = 1;
    }
    int chunk = (high-low)/numThreads;
    // creating the threads and setting their arguements
    for (int i=0; i<numThreads; i++) {
        args[i].low = i*chunk;
        // handling case if size is not multiple of threads
        if(i==numThreads-1){
            args[i].high = high;}
        else{
            args[i].high = (i+1)*chunk;}

        args[i].lambda = lambda;
        if (pthread_create(&tid[i], NULL, thread_func, (void*)(&args[i])) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
        }
    }
    // joining the threads
    for (int i=0; i<numThreads; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
        }
    }
    // priting the time
    auto done = std::chrono::high_resolution_clock::now();
    std :: cout << "\ntime in milli seconds for thread " <<thread_cnt <<": "  ;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count();
    std::cout << "millis seconds\n";
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();
    t_time+=duration;
}

int main(int argc, char **argv) {

    int x=5,y=1;
    // Declaring a lambda expression that accepts void type parameter
    auto  lambda1 = [ x, &y](void) {
        y = 5;
        std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";

    };
    // Executing the lambda function
    demonstration(lambda1);
    int rc = user_main(argc, argv);


    auto  lambda2 = []() {
        std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    };
    demonstration(lambda2);
    std :: cout << "\nThe total time for the execution :" << t_time << " millis \n";
    return rc;
}
#define main user_main