#include "exception_correctness.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

static sigjmp_buf env;
static bool handler_executed = false;

static void custom_handler(int sig) {
    handler_executed = true;
    siglongjmp(env, 1); // Return to checkpoint
}

void test_exception_correctness() {
    volatile int* ptr = NULL;
    struct sigaction sa;
    sa.sa_handler = custom_handler;
    sigaction(SIGSEGV, &sa, NULL);

    if (sigsetjmp(env, 1) == 0) {
        *ptr = 42; // Trigger exception
        printf("Error: Continued after fault\n");
    } else {
        printf("Handler %s\n",
               handler_executed ? "executed correctly" : "failed to execute");
    }

    // Test state persistence
    int state = 0;
    *(&state) = 0xDEADBEEF; // Shouldn't crash
    printf("Post-handler state: 0x%x\n", state);
}
