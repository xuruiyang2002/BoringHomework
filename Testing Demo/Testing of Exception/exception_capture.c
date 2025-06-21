#include "exception_capture.h"
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#define NUM_TRIGGERS 10000

static atomic_int capture_count = 0;
static jmp_buf env;

static void access_handler(int sig) {
    atomic_fetch_add(&capture_count, 1);
    longjmp(env, 1); // Jump to setjmp point
}

void test_exception_capture() {
    volatile int* ptr = NULL;
    struct sigaction sa;

    // Configure signal handler using sigaction
    sa.sa_handler = access_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    for (int i = 0; i < NUM_TRIGGERS; i++) {
        if (setjmp(env) == 0) {
            // Attempt illegal write (triggers SIGSEGV)
            *ptr = 42;
        } else {
            // After returning from handler: manually unblock SIGSEGV
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGSEGV);
            if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
                perror("sigprocmask");
                exit(1);
            }
        }
    }

    printf("Exception capture rate: %.2f%% (%d/%d)\n",
           (float)capture_count * 100 / NUM_TRIGGERS,
           capture_count, NUM_TRIGGERS);
}
