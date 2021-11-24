#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "stopwatch.h"
#include "packetsource.h"
#include "fingerprint.h"
#include "string.h"
#include "pthread.h"

StopWatch_t s_time;
StopWatch_t p_time;
StopWatch_t sq_time;

struct thread_args {
    struct queue* q;
    int T;
    int tid;
    long ** results;
    int N;
};

void print_results(long** results, int n, int p){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < p; j++){
            printf("%ld ", results[i][j]);
        }
        printf("\n");
    }
}

long** serial(int type, int N, int T, long W, short seed){

    PacketSource_t *packetSource = createPacketSource(W, N, seed);
    long * vals = calloc(N*T, sizeof(long));
    long ** results = malloc(sizeof(long*) * N);
    for(int i = 0; i < N; i++){
        results[i] = vals + i*T;
    }

    startTimer(&s_time);
    for(int i = 0; i < T; i++){
        for(int j = 0; j < N; j++){
            volatile Packet_t *packet;
            if(type == 0){
                packet = getConstantPacket(packetSource, j, W);
            }
            else if(type == 1){
                packet = getUniformPacket(packetSource, j);
            }
            else{
                packet = getExponentialPacket(packetSource, j);
            }
            long res = getFingerprint(packet->iterations, packet->seed);
            results[j][i] = res;
        }
    }
    stopTimer(&s_time);

    return results;

}

void* worker_thread(void * args){
    struct thread_args* t_args = (struct thread_args*) args;
    struct queue* q = t_args->q;
    long** results = t_args->results;
    int T = t_args->T;
    int tid = t_args->tid;
    int N = t_args->N;

    for(int i = 0; i < T; i++){
        volatile Packet_t *packet = (Packet_t *) dequeue(q, N);
        long res = getFingerprint(packet->iterations, packet->seed);
        results[tid][i] = res;
    }

    pthread_exit(NULL);

}

long** parallel(int type, int N, int D, int T, long W, short seed){

    // initialize packet sources and results array
    PacketSource_t *packetSource = createPacketSource(W, N, seed);
    long * vals = calloc(N*T, sizeof(long));
    long ** results = malloc(sizeof(long*) * N);
    for(int i = 0; i < N; i++){
        results[i] = vals + i*T;
    }

    // initialize queues
    struct queue* q[N];
    for(int i = 0; i <N; i++){
        q[i] = (struct queue *)malloc(sizeof(struct queue));
        queue_init(q[i], D);
    }

    // initialize thread arguments
    struct thread_args* t_args[N];
    for(int i = 0; i < N; i++){
        t_args[i] = (struct thread_args*)malloc(sizeof(struct thread_args));
        t_args[i]->q = q[i];
        t_args[i]->T = T;
        t_args[i]->tid = i;
        t_args[i]->results = results;
        t_args[i]->N = N;
    }

    startTimer(&p_time);
    // create worker threads
    pthread_t threads[N];
    for(int i = 0; i < N; i++){
        pthread_create(&threads[i], NULL, worker_thread, (void *)t_args[i]);
    }

    // enqueue packets
    for(int i = 0; i < T; i++){
        for(int j = 0; j < N; j++){
            volatile Packet_t *packet;
            if(type == 0){
                packet = getConstantPacket(packetSource, j, W);
            }
            else if(type == 1){
                packet = getUniformPacket(packetSource, j);
            }
            else{
                packet = getExponentialPacket(packetSource, j);
            }
            enqueue(q[j], (void *) packet, N);
        }
    }

    // wait for threads to finish
    for(int i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
    }
    stopTimer(&p_time);
    return results;

}

long ** serial_queue(int type, int N, int D, int T, long W, short seed){
    // initialize packet sources and results array
    PacketSource_t *packetSource = createPacketSource(W, N, seed);
    long * vals = calloc(N*T, sizeof(long));
    long ** results = malloc(sizeof(long*) * N);
    for(int i = 0; i < N; i++){
        results[i] = vals + i*T;
    }

    // initialize queues
    struct queue* q[N];
    for(int i = 0; i <N; i++){
        q[i] = (struct queue *)malloc(sizeof(struct queue));
        queue_init(q[i], D);
    }

    startTimer(&sq_time);
    // enqueue and dequeue packets
    for(int i = 0; i < T; i++){
        for(int j = 0; j < N; j++){
            volatile Packet_t *packet;
            if(type == 0){
                packet = getConstantPacket(packetSource, j, W);
            }
            else if(type == 1){
                packet = getUniformPacket(packetSource, j);
            }
            else{
                packet = getExponentialPacket(packetSource, j);
            }
            enqueue(q[j], (void *) packet, N);

            //dequeue the packet
            volatile Packet_t *p = (Packet_t *) dequeue(q[j], N);
            long res = getFingerprint(p->iterations, p->seed);
            results[j][i] = res;
        }
    }
    stopTimer(&sq_time);

    return results;
}

void print_timer(){
    FILE * fp;
    fp = fopen("results.txt", "w+");
    fprintf(fp, "%f\n%f\n%f\n", getElapsedTime(&s_time), getElapsedTime(&p_time), getElapsedTime(&sq_time));
    fclose(fp);
}

int main(int argc, char* argv[]){
    if(argc != 8){
        printf("proper usage is as follows: ./packet [option] [number of threads] [number of packets] [size of queue] [work per thread] [seed] [type of packets]\n");
        printf("options:\n");
        printf("-s = run serial\n");
        printf("-p = run parallel\n");
        printf("-sq = run serial with queue\n");
        printf("-ct = run all version and check that results match\n");
        printf("-pt = run a performance test and print the results to results.txt");
        return -1;
    }

    int type = atoi(argv[7]);
    int N = atoi(argv[2]);
    int T = atoi(argv[3]);
    int D = atoi(argv[4]);
    long W = (long)atoi(argv[5]);
    short seed = atoi(argv[6]);

    if(strcmp("-s", argv[1]) == 0){
        printf("Running serial implementation for %d packet sources, %d packets, with %ld work\n", N-1, T, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results = serial(type, N-1, T, W, seed);
        print_results(results, N-1, T);
        return 0;
    }
    else if(strcmp("-p", argv[1]) == 0){
        printf("Running parallel implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results = parallel(type, N-1, D, T, W, seed);
        print_results(results, N-1, T);
        return 0;
    }
    else if(strcmp("-sq", argv[1]) == 0){
        printf("Running serial queue implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results = serial_queue(type, N-1, D, T, W, seed);
        print_results(results, N-1, T);
        return 0;
    }
    else if(strcmp("-ct", argv[1]) == 0){
        printf("Running serial implementation for %d packet sources, %d packets, with %ld work\n", N-1, T, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results1 = serial(type, N-1, T, W, seed);

        printf("Running parallel implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results2 = parallel(type, N-1, D, T, W, seed);

        printf("Running serial queue implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        long **results3 = serial_queue(type, N-1, D, T, W, seed);

        for(int i = 0; i < N-1; i++){
            for(int j = 0; j < T; j++){
                long r1 = results1[i][j];
                long r2 = results2[i][j];
                long r3 = results3[i][j];

                if(r1 != r2 || r2 != r3){
                    printf("output do not match!\n");
                    return -1;
                }
            }
        }
        printf("test passed\n");
        return 1;

    }
    else if(strcmp("-pt", argv[1]) == 0){
        printf("Running serial implementation for %d packet sources, %d packets, with %ld work\n", N-1, T, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        serial(type, N-1, T, W, seed);

        printf("Running parallel implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        parallel(type, N-1, D, T, W, seed);

        printf("Running serial queue implementation for %d packet sources and %d packets with queue size = %d and with work = %ld\n", N-1, T, D, W);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        serial_queue(type, N-1, D, T, W, seed);

        print_timer();
        return 1;

    }
    else{
        printf("proper usage is as follows: ./packet [option] [number of threads] [number of packets] [size of queue] [work per thread] [type of packets]\n");
        printf("options:\n");
        printf("-s = run serial\n");
        printf("-p = run parallel\n");
        printf("-sq = run serial with queue\n");
        return -1;
    }


}