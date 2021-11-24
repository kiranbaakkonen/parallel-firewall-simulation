#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "unistd.h"

void queue_init(struct queue *q, int capacity){
    q->arr = malloc(sizeof(void*) * capacity);
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
}

void enqueue(struct queue *q, void * data, int N){
    int head = q->head;
    while(__sync_bool_compare_and_swap(&head, q->tail - q->capacity, q->head)){
        if(N > 15){
            usleep(1);
        }
    };

    q->arr[q->tail % q->capacity] = data;
    __sync_fetch_and_add(&q->tail, 1);
}

void * dequeue(struct queue *q, int N){
    int tail = q->tail;
    while(__sync_bool_compare_and_swap(&tail, q->head, q->tail)){
        if(N > 15){
            usleep(1);
        }
    };

    void * res = q->arr[q->head % q->capacity];
    __sync_fetch_and_add(&q->head, 1);
    return res;
}
