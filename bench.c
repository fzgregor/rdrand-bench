#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include "rdrand.h"
#include "rdtsc.h"

const int SIZE = 256*1024*1024;
const int NANO_SECONDS_IN_SEC = 1000000000;

pthread_barrier_t barrier;

typedef struct thread_state_s {
} thread_state_t;

void *thread_run_crc32_rand(void *tstate_void) {
    thread_state_t *tstate = (thread_state_t*)tstate_void;

    pthread_barrier_wait(&barrier);
    uint64_t i = 0;
    volatile uint32_t rand;
    for (; i < SIZE; i+=4) {
        rand = crc32_rand();
    }

    return NULL;
}

void *thread_run_rdrand32(void *tstate_void) {
    thread_state_t *tstate = (thread_state_t*)tstate_void;

    pthread_barrier_wait(&barrier);
    uint64_t i = 0;
    volatile uint32_t rand;
    for (; i < SIZE; i+=4) {
        rand = real_rdrand32();
    }

    return NULL;
}

void *thread_run_rdrand64(void *tstate_void) {
    thread_state_t *tstate = (thread_state_t*)tstate_void;

    pthread_barrier_wait(&barrier);
    uint64_t i = 0;
    volatile uint64_t rand;
    for (; i < SIZE; i+=8) {
        rand = rdrand64();
    }

    return NULL;
}

void do_bench(uint32_t tcnt, bool csv_out, pthread_t* tids, thread_state_t* tstates, char *name, void*(*c)(void*)) {
    pthread_barrier_init(&barrier, NULL, tcnt+1);

    uint32_t i;
    for (i=0; i < tcnt; i++) {
        pthread_create(&tids[i], NULL, c, (void*)&tstates[i]);
    }

    uint64_t begin = rdtsc();
    pthread_barrier_wait(&barrier);

    for (i=0; i < tcnt; i++) {
        pthread_join(tids[i], NULL);
    }
    double time = elapsed_nsecs(begin)/NANO_SECONDS_IN_SEC;
    uint64_t size = SIZE/1024/1024;

    if (csv_out) {
        printf("%s,%d,%f,%f\n", name, tcnt, time, tcnt*size/time);
    } else {
        printf("%s: %d Threads in %f secs %f MB/s\n", name, tcnt,  time, tcnt*size/time);
    }

    pthread_barrier_destroy(&barrier);
}

int main(int argc, char* argv[]) {
    int opt;

    uint32_t max_tcnt = 1;
    bool csv_out = false;

    while ((opt = getopt(argc, argv, "t:ch")) != -1) {
        switch(opt) {
        case 't':
            max_tcnt = strtol(optarg, NULL, 10);
            // TODO: error checking
            break;
        case 'c':
            csv_out = true;
            break;
        case 'h':
            fprintf(stderr,"Benchmark rdrand instruction\n-t n\tmaximal number of threads\n-c\toutput in CSV format\n");
            exit(1);
            break;
        default:
            break;
        }
    }

    uint64_t i = 0;
    thread_state_t* tstates = (thread_state_t*)calloc(sizeof(thread_state_t), max_tcnt);
    pthread_t* tids = (pthread_t*)calloc(sizeof(pthread_t), max_tcnt);

    if (csv_out) {
        printf("name,threads,seconds,throughput\n");
    }

    for (i=1; i <= max_tcnt; i++) {
        do_bench(i, csv_out, tids, tstates, "crc32_rand", thread_run_crc32_rand);
    }

    for (i=1; i <= max_tcnt; i++) {
        do_bench(i, csv_out, tids, tstates, "rdrand32", thread_run_rdrand32);
    }

    for (i=1; i <= max_tcnt; i++) {
        do_bench(i, csv_out, tids, tstates, "rdrand64", thread_run_rdrand64);
    }
}
