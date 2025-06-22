#include "exception_correctness.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

static jmp_buf env;
static bool handler_executed = false;

static void custom_handler(int sig) {
    handler_executed = true;
    longjmp(env, 1);  // Use normal longjmp
}

void test_exception_correctness() {
    volatile int* ptr = NULL;
    struct sigaction sa;
    sa.sa_handler = custom_handler;
    sigemptyset(&sa.sa_mask);  // No additional signals blocked during handler
    sa.sa_flags = 0;

    // Register handler properly
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return;
    }

    // Set checkpoint (replace sigsetjmp)
    if (setjmp(env) == 0) {
        *ptr = 42;  // Trigger SIGSEGV
        // Should never reach here
        printf("Error: Continued after fault\n");
    } else {
        // After longjmp: manually unblock SIGSEGV
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        printf("Handler %s\n",
               handler_executed ? "executed correctly" : "failed to execute");
    }

    // Validate state persistence
    int state = 0;
    *(&state) = 0xDEADBEEF;  // Valid write (stack address)
    printf("Post-handler state: 0x%x\n", state);
}
