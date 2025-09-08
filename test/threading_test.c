#include <pthread.h>            // for pthread_join, pthread_create, pthread_t
#include <stdbool.h>            // for bool, false, true
#include <stdint.h>             // for uintptr_t
#include <string.h>             // for memset
#include <unistd.h>             // for NULL, usleep

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace, bw_backtrace_cb

#include "test.h"               // for TEST_ERROR_NONZERO, TEST, TEST_ASSERT...

enum { MAX_THREADS = 8 };
enum { ITERATIONS_PER_THREAD = 100 };

// Thread-safe counter callback
bool thread_safe_counter(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    volatile int* count = (volatile int*)arg;
    __sync_fetch_and_add(count, 1); // Atomic increment

    return true;
}

// Callback that tracks thread-specific data
bool track_thread_data(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);

    struct thread_stats {
        int total_calls;
        int valid_fnames;
        int valid_snames;
        pthread_t thread_id;
    }* stats = (struct thread_stats*)arg;

    __sync_fetch_and_add(&stats->total_calls, 1);

    if (fname && fname[0] != '\0') {
        __sync_fetch_and_add(&stats->valid_fnames, 1);
    }

    if (sname && sname[0] != '\0') {
        __sync_fetch_and_add(&stats->valid_snames, 1);
    }

    return true;
}

// Thread data structure
typedef struct {
    int thread_id;
    int iterations;
    int frames_found;
    bool success;
    volatile int* shared_counter;
} thread_data_t;

// Basic backtrace thread function
__attribute__((noinline)) void* backtrace_thread_func(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    const int delay_usecs = 1000;

    for (int i = 0; i < data->iterations; i++) {
        int local_count = 0;
        bool result = bw_backtrace(thread_safe_counter, &local_count);

        if (!result) {
            data->success = false;
            return NULL;
        }

        data->frames_found += local_count;

        // Small delay to increase chance of thread interleaving
        BW_UNUSED(usleep(delay_usecs)); // 1ms
    }

    data->success = true;
    return NULL;
}

// Recursive thread function to create deeper stacks
// NOLINTNEXTLINE(misc-no-recursion)
__attribute__((noinline)) void* recursive_thread_helper(void* arg, int depth) {
    if (depth <= 0) {
        return backtrace_thread_func(arg);
    }

    return recursive_thread_helper(arg, depth - 1);
}

__attribute__((noinline)) void* recursive_backtrace_thread(void* arg) {
    const int recursion_depth = 5;
    return recursive_thread_helper(arg, recursion_depth); // 5 levels of recursion
}

// Function pointer thread to test indirect calls
__attribute__((noinline)) void* function_pointer_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;

    // Use function pointer for backtrace call
    bool (*backtrace_func)(bw_backtrace_cb, void*) = bw_backtrace;

    for (int i = 0; i < data->iterations; i++) {
        int local_count = 0;
        bool result = backtrace_func(thread_safe_counter, &local_count);

        if (!result) {
            data->success = false;
            return NULL;
        }

        data->frames_found += local_count;
    }

    data->success = true;
    return NULL;
}

TEST(basic_multithreaded_backtrace, {
    const int num_threads = 4;
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    volatile int shared_counter = 0;

    // Initialize thread data
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = 20;
        thread_data[i].frames_found = 0;
        thread_data[i].success = false;
        thread_data[i].shared_counter = &shared_counter;
    }

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        int retval = pthread_create(&threads[i], NULL, backtrace_thread_func, &thread_data[i]);
        TEST_ERROR_NONZERO(retval);
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        TEST_ERROR_NONZERO(pthread_join(threads[i], NULL));

        TEST_ASSERT_TRUE(thread_data[i].success);
        TEST_ASSERT_GE_INT32(thread_data[i].frames_found, thread_data[i].iterations);
    }
})

TEST(recursive_multithreaded_backtrace, {
    const int num_threads = 3;
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];

    // Initialize thread data
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = 10;
        thread_data[i].frames_found = 0;
        thread_data[i].success = false;
        thread_data[i].shared_counter = NULL;
    }

    // Create threads with recursive calls
    for (int i = 0; i < num_threads; i++) {
        int retval = pthread_create(&threads[i], NULL, recursive_backtrace_thread, &thread_data[i]);
        TEST_ERROR_NONZERO(retval);
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        TEST_ERROR_NONZERO(pthread_join(threads[i], NULL));
        TEST_ASSERT_TRUE(thread_data[i].success);
    }
})

TEST(function_pointer_multithreaded, {
    const int num_threads = 2;
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];

    // Initialize thread data
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = 15;
        thread_data[i].frames_found = 0;
        thread_data[i].success = false;
        thread_data[i].shared_counter = NULL;
    }

    // Create threads using function pointers
    for (int i = 0; i < num_threads; i++) {
        int retval = pthread_create(&threads[i], NULL, function_pointer_thread, &thread_data[i]);
        TEST_ERROR_NONZERO(retval);
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        TEST_ERROR_NONZERO(pthread_join(threads[i], NULL));
        TEST_ASSERT_TRUE(thread_data[i].success);
    }
})

// High contention test with many threads
TEST(high_contention_backtrace, {
    const int num_threads = 8;
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];

    // Initialize thread data for high contention
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = 50;
        thread_data[i].frames_found = 0;
        thread_data[i].success = false;
        thread_data[i].shared_counter = NULL;
    }

    // Create many threads simultaneously
    for (int i = 0; i < num_threads; i++) {
        int retval = pthread_create(&threads[i], NULL, backtrace_thread_func, &thread_data[i]);
        TEST_ERROR_NONZERO(retval);
    }

    // Join all threads
    int successful_threads = 0;
    for (int i = 0; i < num_threads; i++) {
        TEST_ERROR_NONZERO(pthread_join(threads[i], NULL));

        if (thread_data[i].success) {
            successful_threads++;
        }
    }

    // At least most threads should succeed
    TEST_ASSERT_GE_INT32(successful_threads, num_threads - 1);
})

// Test thread-local data collection
void* data_collection_thread(void* arg) {
    struct thread_stats {
        int total_calls;
        int valid_fnames;
        int valid_snames;
        pthread_t thread_id;
        bool success;
    }* stats = (struct thread_stats*)arg;

    stats->thread_id = pthread_self();
    stats->success = true;

    // Do several backtraces in this thread
    const int num_backtraces = 5;
    for (int i = 0; i < num_backtraces; i++) {
        bool result = bw_backtrace(track_thread_data, stats);
        if (!result) {
            stats->success = false;
            break;
        }
    }

    return NULL;
}

TEST(thread_local_data_collection, {
    const int num_threads = 4;
    pthread_t threads[MAX_THREADS];
    struct thread_stats {
        int total_calls;
        int valid_fnames;
        int valid_snames;
        pthread_t thread_id;
        bool success;
    } stats[MAX_THREADS];

    // Initialize stats
    for (int i = 0; i < num_threads; i++) {
        BW_UNUSED(memset(&stats[i], 0, sizeof(stats[i])));
    }

    // Create data collection threads
    for (int i = 0; i < num_threads; i++) {
        int retval = pthread_create(&threads[i], NULL, data_collection_thread, &stats[i]);
        TEST_ERROR_NONZERO(retval);
    }

    // Join threads and verify data
    for (int i = 0; i < num_threads; i++) {
        TEST_ERROR_NONZERO(pthread_join(threads[i], NULL));

        TEST_ASSERT_TRUE(stats[i].success);

        // Verify thread collected some data
        TEST_ASSERT_GE_INT32(stats[i].total_calls, 5); // At least 5 calls * frames per call
        TEST_ASSERT_TRUE(stats[i].thread_id != 0); // Thread ID should be set
    }
})

int main(int argc, char** argv) {
    TEST_INIT("threading", argc, argv);

    TEST_RUN(basic_multithreaded_backtrace);
    TEST_RUN(recursive_multithreaded_backtrace);
    TEST_RUN(function_pointer_multithreaded);
    TEST_RUN(high_contention_backtrace);
    TEST_RUN(thread_local_data_collection);

    TEST_EXIT();
}
