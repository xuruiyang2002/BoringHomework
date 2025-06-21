#define _POSIX_C_SOURCE 199309L
#include "exception_time.h"
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <setjmp.h>

#define NUM_SAMPLES 1000

static atomic_bool start_timing = false;
static struct timespec raise_time;
static jmp_buf env;  // Use normal jmp_buf instead of sigjmp_buf

static atomic_int exception_count = 0;
static atomic_llong total_latency = 0;

static void access_handler(int sig) {
    if (!atomic_load(&start_timing)) return;

    struct timespec handler_time;
    clock_gettime(CLOCK_MONOTONIC, &handler_time);
    long latency = (handler_time.tv_sec - raise_time.tv_sec) * 1000000000L +
                  (handler_time.tv_nsec - raise_time.tv_nsec);
    atomic_fetch_add(&total_latency, latency);
    atomic_fetch_add(&exception_count, 1);
    longjmp(env, 1);  // Use normal longjmp instead of siglongjmp
}

void test_exception_response_time() {
    volatile int* ptr = NULL;
    struct sigaction sa;

    // Configure signal handler
    sa.sa_handler = access_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction failed");
        return;
    }

    atomic_store(&start_timing, true);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (setjmp(env) == 0) {
            clock_gettime(CLOCK_MONOTONIC, &raise_time);
            *ptr = 42;  // Trigger exception
        } else {
            // After escaping from handler: unblock SIGSEGV
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &set, NULL);
        }
    }

    if (exception_count > 0) {
        printf("Average exception latency: %lld ns\n",
               atomic_load(&total_latency) / atomic_load(&exception_count));
    } else {
        printf("No exceptions captured!\n");
    }
}
