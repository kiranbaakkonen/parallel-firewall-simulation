#include <stdlib.h>
#include <stdio.h>
#include "pthread.h"
#include <unistd.h>

struct queue {
    int * arr;
    int capacity;
    volatile int head;
    volatile int tail;
};

void queue_init(struct queue *q, int capacity){
    q->arr = (int *)malloc(sizeof(int) * capacity);
    q->capacity = capacity;
    q->head = 0; 
    q->tail = 0;
}

void enqueue(struct queue *q, int data){
    int head = q->head;
    while(__sync_bool_compare_and_swap(&head, q->tail - q->capacity, q->head)){};

    q->arr[q->tail % q->capacity] = data;
    __sync_fetch_and_add(&q->tail, 1);
}

int dequeue(struct queue *q){
    int tail = q->tail;
    while(__sync_bool_compare_and_swap(&tail, q->head, q->tail)){};

    int res = q->arr[q->head % q->capacity];
    __sync_fetch_and_add(&q->head, 1);
    return res;
}

struct thread_args {
    struct queue* q;
    int iter;
    int tid;
};

void* dequeuer_thread(void * args){

    struct thread_args* t_args = (struct thread_args*) args;
    struct queue* q = t_args->q;
    int iter = t_args->iter;

    for(int i = 0; i < iter; i++){
        int e = dequeue(q);
        if(e != i){ 
            printf("wrong order: ");
        }
        printf("%d\n", e);
    }

    pthread_exit(NULL);
}

void* dequeuer_thread_multiple(void * args){

    struct thread_args* t_args = (struct thread_args*) args;
    struct queue* q = t_args->q;
    int iter = t_args->iter;

    for(int i = 0; i < iter; i++){
        int e = dequeue(q);
        int sleep_time = (rand() % 100) * t_args->tid * 10;
        usleep(sleep_time);
        if(e != i){ 
            printf("wrong order: ");
        }
        printf("%d\n", e);
    }

    pthread_exit(NULL);
}

void test_single_queue(){

    printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("testing a single queue with 2 threads of size = 25\nwill enqueue 10000 elements and then dequeue them\n\n");

    // initialize queue of size 100
    struct queue* q = (struct queue *)malloc(sizeof(struct queue));
    queue_init(q, 25);
    
    printf("initializing queue\n");
    printf("enqueue and dequeue elements (50 elements should appear in ascending order)\n-------------------\n");

    // create thread args
    struct thread_args* t_args = (struct thread_args*)malloc(sizeof(struct thread_args));
    t_args->q = q;
    t_args->iter = 10000;
    t_args->tid = 1;

    //create dequeuer thread
    pthread_t thread;
    pthread_create(&thread, NULL, dequeuer_thread, t_args);

    for(int i = 0; i < t_args->iter; i++){
        enqueue(q, i);
    }

    pthread_join(thread, NULL);
}



void test_multiple_queues(int N, int T){
    printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("testing a 10 queues (no threads) of size = 25\nwill enqueue 50 elements to each queue and then dequeue them\n");

    // initialize queues of size 25
    struct queue* q[N];
    for(int i = 0; i < N; i++){
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
        t_args[i]->tid = i + 1;
    }

    // create dequeuer threads
    pthread_t threads[N];
    for(int i = 0; i < N; i++){
        pthread_create(&threads[i], NULL, dequeuer_thread_multiple, (void*)t_args[i]);
    }

    for(int i = 0; i < T; i++){
        for(int j = 0; j < N; j++){
            enqueue(q[j], i);
        }
    }

    for(int i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
        printf("thread %d finished successfully\n", i+1);
    }
}

int main(){
    //test_single_queue();
    test_multiple_queues(10, 100000);
}