#define _POSIX_C_SOURCE 199309L
#include "memory_collision.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>
#include <stdint.h>

#define CACHE_LINE_SIZE 64
#define NUM_ACCESSES 100000

/*
 Uses cache-aligned shared buffer to force memory bus contention
 Measures individual access latency using fine-grained timing
 atomic_signal_fence prevents compiler optimizations
 Reports average access latency (higher = more collisions)
 */

// Shared buffer (aligned to cache line)
volatile uint8_t shared_buffer[CACHE_LINE_SIZE] __attribute__((aligned(CACHE_LINE_SIZE)));

void test_memory_collision_rate() {
    struct timespec start, end;
    long total_duration = 0;

    for (int i = 0; i < NUM_ACCESSES; i++) {
        clock_gettime(CLOCK_REALTIME, &start);

        // Force memory access (write to shared buffer)
        shared_buffer[(i * 7) % CACHE_LINE_SIZE] = i & 0xFF;
        atomic_signal_fence(memory_order_seq_cst); // Prevent compiler reordering

        clock_gettime(CLOCK_REALTIME, &end);
        total_duration += (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    }

    printf("Avg memory access latency: %ld ns\n", total_duration / NUM_ACCESSES);
}
