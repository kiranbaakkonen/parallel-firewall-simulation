#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>

/*
struct elem {
    struct elem *next;
    volatile void *data;
};

struct queue{
    volatile int head;
    volatile int tail;
    struct elem *start;
    struct elem *end;
    int size;
    int guard;
    
};

*/

struct queue{
    void ** arr;
    int capacity;
    volatile int head;
    volatile int tail;
};

void queue_init(struct queue* q, int capacity);
void * dequeue(struct queue* q, int N);
void enqueue(struct queue* q, void * data, int N);

#endif

