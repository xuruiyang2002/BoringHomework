#define _POSIX_C_SOURCE 199309L
#include "memory_isolation.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define BUFFER_SIZE (100 * 1024 * 1024) // 100MB buffer

void test_memory_isolation() {
    uint8_t *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc failed");
        return;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    
    // Write to entire buffer
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = i & 0xFF;
    }
    
    clock_gettime(CLOCK_REALTIME, &end);
    long duration = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("Memory write time: %ld ns\n", duration);
    
    free(buffer);
}