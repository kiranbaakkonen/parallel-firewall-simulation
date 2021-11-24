#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "pthread.h"

struct thread_args {
    struct queue* q;
    int iter;
    int N;
};

void* dequeuer_thread(void * args){

    struct thread_args* t_args = (struct thread_args*) args;
    struct queue* q = t_args->q;
    int iter = t_args->iter;
    int N = t_args->N;

    for(int i = 0; i < iter; i++){
        int e = *(int*) dequeue(q, N);
        if(e != i){
            printf("wrong order:");
        }
        printf("%d\n", e);
    }

    pthread_exit(NULL);
}

void test_single_queue(int iter){

    printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("testing a single queue with 2 threads of size = 25\nwill enqueue %d elements and then dequeue them\n\n", iter);

    // initialize queue of size 100
    struct queue* q = (struct queue *)malloc(sizeof(struct queue));
    queue_init(q, 25);
    
    printf("initializing queue\n");
    printf("enqueue and dequeue elements (warning will occur if elements are in the wrong order)\n-------------------\n");

    // create thread args
    struct thread_args* t_args = (struct thread_args*)malloc(sizeof(struct thread_args));
    t_args->q = q;
    t_args->iter = iter;
    t_args->N = 1;

    int data[iter];
    for(int i = 0; i < iter ; i++){
        data[i] = i;
    }

    //create dequeuer thread
    pthread_t thread;
    pthread_create(&thread, NULL, dequeuer_thread, t_args);

    for(int i = 0; i < t_args->iter; i++){
        enqueue(q, (void *) &data[i], 1);
    }

    pthread_join(thread, NULL);
    printf("test completed\n\n");
}

void test_single_queue_no_threads(){

    printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("testing a single queue (no threads) of size = 25\nwill enqueue 25 elements and then dequeue them twice\n");

    // initialize queue of size 25
    struct queue* q = (struct queue *)malloc(sizeof(struct queue));
    queue_init(q, 25);

    printf("initializing queue\n");
    printf("enqueue and dequeue elements (50 elements should appear in ascending order\n-------------------\n");

    int data[50];
    for(int i = 0; i < 50 ; i++){
        data[i] = i;
    }

    // add 25 items to the queue
    for(int i = 0; i < 25; i++){
       enqueue(q, (void*) &data[i], 1);
    }

    // remove the 25 items from the queue
    for(int i = 0; i < 25; i++){
        int e = *(int*) dequeue(q, 1);
        if(e != i){
            printf("wrong order: ");
        }
        printf("%d\n", e);
    }

    // add 25 more items to the queue
    for(int i = 0; i < 25; i++){
        enqueue(q, (void*) &data[i+25], 1);
    }

    // remove 25 more items
    for(int i = 0; i < 25; i++){
        int e = *(int*) dequeue(q, 1);
        if(e != i+25){
            printf("wrong order: ");
        }
        printf("%d\n", e);
    }
    printf("test completed\n\n");

}

void* dequeuer_thread_multiple(void * args){

    struct thread_args* t_args = (struct thread_args*) args;
    struct queue* q = t_args->q;
    int iter = t_args->iter;
    int N = t_args->N;

    for(int i = 0; i < iter; i++){
        int e = *(int*) dequeue(q, N);
        if(e != i){
            printf("element in wrong order\n");
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}

void test_multiple_queues(int N, int T){
    printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("testing %d queues of size = 25\nwill enqueue %d elements to each queue and then dequeue them\n", N, T);

    // initialize queues of size 25
    struct queue* q[N];
    for(int i = 0; i <N; i++){
        q[i] = (struct queue *)malloc(sizeof(struct queue));
        queue_init(q[i], 25);
    }
    
    printf("initializing queues\n");
    printf("enqueue and dequeue elements (elements will not be printed)\n-------------------\n");

    // create thread args
    struct thread_args* t_args[N];
    for(int i = 0; i < N; i++){
        t_args[i] = (struct thread_args*)malloc(sizeof(struct thread_args));
        t_args[i]->q = q[i];
        t_args[i]->iter = T;
        t_args[i]->N = N;
    }   

    int data[T];
    for(int i = 0; i < T ; i++){
        data[i] = i;
    }

    // create dequeuer threads
    pthread_t threads[N];
    for(int i = 0; i < N; i++){
        pthread_create(&threads[i], NULL, dequeuer_thread_multiple, (void*)t_args[i]);
    }

    for(int i = 0; i < T; i++){
        for(int j = 0; j < N; j++){
           enqueue(q[j], (void*) &data[i], N);

        }
    }

    for(int i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
        printf("thread %d finished successfully\n", i+1);
    }

    printf("test completed\n\n");

}

int main(int argc, char* argv[]){

    if(argc != 2){
        printf("Tester usage:\n");
        printf("./test_queue [option]\n");
        printf("options:\n");
        printf("0 = run test with no threads\n");
        printf("1 = run test with two threads\n");
        printf("2 = run test with 10 threads\n");
        printf("3 = run all tests\n");
        return 0;
    }

    if(atoi(argv[1]) == 0){
        test_single_queue_no_threads();
    }   
    else if(atoi(argv[1]) == 1){
        test_single_queue(100);
    }
    else if(atoi(argv[1]) == 2){
        test_multiple_queues(10, 10000);
        return 0;
    }
    else if(atoi(argv[1]) == 3){
        test_single_queue_no_threads();
        test_single_queue(100);
        test_multiple_queues(10, 10000);
        return 0;
    }
    else{
        printf("Tester usage:\n");
        printf("./test_queue [option]\n");
        printf("options:\n");
        printf("0 = run test with no threads\n");
        printf("1 = run test with two threads\n");
        printf("2 = run test with 10 threads\n");
        printf("3 = run all tests\n");
    }
    return 0;
}