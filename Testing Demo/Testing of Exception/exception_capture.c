#include "exception_capture.h"
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#define NUM_TRIGGERS 10000

static atomic_int capture_count = 0;
static sigjmp_buf env;

// Triggers controlled memory access faults
// Uses signal handler to count captured exceptions
// Tests hypervisor's ability to trap exceptions
static void access_handler(int sig) {
    atomic_fetch_add(&capture_count, 1);
    siglongjmp(env, 1); // Escape to safe point
}

void test_exception_capture() {
    volatile int* ptr = NULL;
    signal(SIGSEGV, access_handler);

    for (int i = 0; i < NUM_TRIGGERS; i++) {
        if (sigsetjmp(env, 1) == 0) {
            *ptr = 42;
        }
    }

    printf("Exception capture rate: %.2f%% (%d/%d)\n",
           (float)capture_count * 100 / NUM_TRIGGERS,
           capture_count, NUM_TRIGGERS);
}
