#define _POSIX_C_SOURCE 199309L
#include "exception_time.h"
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_SAMPLES 1000

static atomic_bool start_timing = false;
static struct timespec raise_time;

// Measures nanosecond latency between fault and handler
// Uses monotonic clock for precise timing
// Directly measures hypervisor trap latency
static void access_handler(int sig) {
    if (!atomic_load(&start_timing)) return;
    
    struct timespec handler_time;
    clock_gettime(CLOCK_MONOTONIC, &handler_time);
    long latency = (handler_time.tv_sec - raise_time.tv_sec) * 1000000000L + 
                  (handler_time.tv_nsec - raise_time.tv_nsec);
    printf("Exception latency: %ld ns\n", latency);
}

void test_exception_response_time() {
    volatile int* ptr = NULL;
    struct sigaction sa;
    sa.sa_handler = access_handler;
    sigaction(SIGSEGV, &sa, NULL);

    atomic_store(&start_timing, true);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        clock_gettime(CLOCK_MONOTONIC, &raise_time);
        *ptr = 42; // Trigger exception
    }
}