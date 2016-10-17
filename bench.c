#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include "rdrand.h"

const int SIZE = 128*1024*1024;
const int NANO_SECONDS_IN_SEC = 1000000000;

pthread_barrier_t barrier;

typedef struct thread_state_s {
    uint8_t *memory;
    uint32_t tsc_ticks;
} thread_state_t;

uint64_t rdtsc() {
    unsigned int a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return (((uint64_t)a) | (((uint64_t)d) << 32));
}

struct timespec *TimeSpecDiff(struct timespec *ts1, struct timespec *ts2)
{
  static struct timespec ts;
  ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
  ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
  if (ts.tv_nsec < 0) {
    ts.tv_sec--;
    ts.tv_nsec += NANO_SECONDS_IN_SEC;
  }
  return &ts;
}

static double calibrate_ticks()
{
  struct timespec begints, endts;
  uint64_t begin = 0, end = 0;
  clock_gettime(CLOCK_MONOTONIC, &begints);
  begin = rdtsc();
  uint64_t i;
  for (i = 0; i < 1000000; i++); /* must be CPU intensive */
  end = rdtsc();
  clock_gettime(CLOCK_MONOTONIC, &endts);
  struct timespec *tmpts = TimeSpecDiff(&endts, &begints);
  uint64_t nsecElapsed = tmpts->tv_sec * 1000000000LL + tmpts->tv_nsec;
  return (double)(end - begin)/(double)nsecElapsed;
}

void *thread_run_rdrand32(void *tstate_void) {
    thread_state_t *tstate = (thread_state_t*)tstate_void;

    pthread_barrier_wait(&barrier);
    uint64_t i = 0;
    for (; i < SIZE; i+=4) {
        real_rdrand32((uint32_t*)&tstate->memory[i]);
    }

    return NULL;
}

void *thread_run_rdrand64(void *tstate_void) {
    thread_state_t *tstate = (thread_state_t*)tstate_void;

    pthread_barrier_wait(&barrier);
    uint64_t i = 0;
    for (; i < SIZE; i+=8) {
        rdrand64((uint64_t*)&tstate->memory[i]);
    }

    return NULL;
}

void do_bench(uint32_t tcnt, bool csv_out, pthread_t* tids, thread_state_t* tstates, char *name, void*(*c)(void*)) {
    pthread_barrier_init(&barrier, NULL, tcnt+1);

    uint32_t i;
    for (i=0; i < tcnt; i++) {
        pthread_create(&tids[i], NULL, c, (void*)&tstates[i]);
    }

    uint64_t tsc_ticks = rdtsc();
    pthread_barrier_wait(&barrier);

    for (i=0; i < tcnt; i++) {
        pthread_join(tids[i], NULL);
    }
    tsc_ticks = rdtsc() - tsc_ticks;
    uint64_t size = SIZE/1024/1024;
    double time = tsc_ticks/calibrate_ticks()/NANO_SECONDS_IN_SEC;

    if (csv_out) {
        printf("%s,%d,%lu,%f,%f\n", name, tcnt, tcnt*size, time, tcnt*size/time);
    } else {
        printf("%s: %d Threads %lu MB in %f secs %f MB/s\n", name, tcnt, tcnt*size, time, tcnt*size/time);
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

    for (; i < max_tcnt; i++) {
        tstates[i].memory = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_POPULATE, -1, 0);
        if (tstates[i].memory == NULL) {
            perror("mmap");
            exit(1);
        }
    }

    if (csv_out) {
        printf("name,threads,size,seconds,throughput\n");
    }

    i = 1;
    for (; i <= max_tcnt; i++) {
        do_bench(i, csv_out, tids, tstates, "rdrand32", thread_run_rdrand32);
    }

    i = 1;
    for (; i <= max_tcnt; i++) {
        do_bench(i, csv_out, tids, tstates, "rdrand64", thread_run_rdrand64);
    }
}
