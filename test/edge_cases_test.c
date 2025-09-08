#include <stdbool.h>            // for bool, true, false
#include <stddef.h>             // for NULL
#include <stdint.h>             // for uintptr_t
#include <string.h>             // for strlen

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace

#include "test.h"               // for TEST, TEST_RUN, TEST_ASSERT_GE_INT32

// Test callback that always returns false (stops immediately)
bool stop_immediately_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    int* call_count = (int*)arg;
    (*call_count)++;

    return false; // Stop immediately
}

// Test callback that tracks null/invalid parameters
bool track_nulls_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);

    struct {
        int total_calls;
        int null_fname;
        int null_sname;
        int empty_fname;
        int empty_sname;
    }* stats = arg;

    stats->total_calls++;

    if (fname == NULL) {
        stats->null_fname++;
    } else if (strlen(fname) == 0) {
        stats->empty_fname++;
    }

    if (sname == NULL) {
        stats->null_sname++;
    } else if (strlen(sname) == 0) {
        stats->empty_sname++;
    }

    return true;
}

// Callback that counts all frames without stopping
bool count_all_frames_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    int* count = (int*)arg;
    (*count)++;

    return true; // Continue counting all frames
}

// Extremely deep recursion to test stack limits
// NOLINTNEXTLINE(misc-no-recursion)
__attribute__((noinline)) bool deep_recursion(int depth, int* max_frames_seen) {
    if (depth <= 0) {
        // At the deepest point, run backtrace
        int frame_count = 0;
        bool success = bw_backtrace(count_all_frames_cb, &frame_count);
        *max_frames_seen = frame_count;
        return success;
    }

    return deep_recursion(depth - 1, max_frames_seen);
}

TEST(immediate_stop_callback, {
    int call_count = 0;

    bool success = bw_backtrace(stop_immediately_cb, &call_count);

    // bw_backtrace returns false when callback returns false (early termination)
    TEST_ASSERT_FALSE(success); // Should return false since callback stops immediately
    TEST_ASSERT_EQ_INT32(call_count, 1); // Should be called exactly once
})

TEST(null_fname_sname_handling, {
    struct {
        int total_calls;
        int null_fname;
        int null_sname;
        int empty_fname;
        int empty_sname;
    } stats = {0};

    bool success = bw_backtrace(track_nulls_cb, &stats);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_INT32(stats.total_calls, 1);

    // Some fname/sname might be null or empty - this is implementation dependent
    // Just ensure we don't crash and get some reasonable data
    TEST_ASSERT_GE_INT32(stats.total_calls, stats.null_fname + stats.empty_fname);
    TEST_ASSERT_GE_INT32(stats.total_calls, stats.null_sname + stats.empty_sname);
})

TEST(very_deep_stack, {
    int max_frames = 0;
    const int recursion_depth = 50;

    // Test with moderately deep recursion (50 levels)
    bool success = deep_recursion(recursion_depth, &max_frames);

    TEST_ASSERT_TRUE(success); // Returns true since count_all_frames_cb counts all frames
    TEST_ASSERT_GE_INT32(max_frames, recursion_depth); // Should see at least the recursive depth
})

// Test with different callback argument types
bool test_different_args_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    // arg could be anything - just test we don't crash
    if (arg != NULL) {
        // Try to access it safely
        volatile char* ptr = (volatile char*)arg;
        BW_UNUSED(*ptr); // This might crash if arg is invalid, but that's expected
    }

    return false; // Stop after first call
}

TEST(null_arg_parameter, {
    // Test with NULL argument
    bool success = bw_backtrace(test_different_args_cb, NULL);
    TEST_ASSERT_FALSE(success); // Returns false since callback returns false
})

TEST(valid_arg_parameter, {
    int dummy_value = 42;

    // Test with valid argument
    bool success = bw_backtrace(test_different_args_cb, &dummy_value);
    TEST_ASSERT_FALSE(success); // Returns false since callback returns false
})

// Test callback that modifies the arg extensively
bool modify_arg_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    const int increment = 100;

    if (arg) {
        int* counter = (int*)arg;
        *counter += increment; // Modify the argument
    }

    return true;
}

TEST(callback_modifies_arg, {
    int value = 5;
    const int expected_increment = 100;

    bool success = bw_backtrace(modify_arg_cb, &value);

    TEST_ASSERT_TRUE(success); // Returns false since callback returns false
    TEST_ASSERT_GE_INT32(value, 5 + expected_increment); // Should be modified by callback
})

// Test with function pointers and indirect calls
typedef bool (*test_func_ptr_t)(void);

__attribute__((noinline)) bool indirect_call_helper(void) {
    int count = 0;
    return bw_backtrace(stop_immediately_cb, &count);
}

__attribute__((noinline)) bool call_through_function_pointer(test_func_ptr_t func) {
    return func();
}

TEST(function_pointer_backtrace, {
    test_func_ptr_t func_ptr = indirect_call_helper;

    bool success = call_through_function_pointer(func_ptr);

    TEST_ASSERT_FALSE(success); // Returns false since stop_immediately_cb returns false
})

int main(int argc, char** argv) {
    TEST_INIT("edge_cases", argc, argv);

    TEST_RUN(immediate_stop_callback);
    TEST_RUN(null_fname_sname_handling);
    TEST_RUN(very_deep_stack);
    TEST_RUN(null_arg_parameter);
    TEST_RUN(valid_arg_parameter);
    TEST_RUN(callback_modifies_arg);
    TEST_RUN(function_pointer_backtrace);

    TEST_EXIT();
}
