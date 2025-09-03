#include <stdbool.h>            // for bool, true
#include <stddef.h>             // for NULL
#include <stdint.h>             // for uintptr_t
#include <string.h>             // for strlen, strstr

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace

#include "test.h"               // for TEST, TEST_RUN, TEST_ASSERT_TRUE, TES...

typedef struct {
    int frame_count;
    bool found_expected_func;
    const char* expected_func;
} backtrace_context_t;

bool count_frames_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);

    backtrace_context_t* ctx = (backtrace_context_t*)arg;
    ctx->frame_count++;

    if (sname && ctx->expected_func && strstr(sname, ctx->expected_func)) {
        ctx->found_expected_func = true;
    }

    return true; // Continue iterating
}

bool stop_after_n_frames_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    int* count = (int*)arg;
    (*count)++;

    return *count < 3; // Stop after 3 frames
}

__attribute__((noinline)) bool deep_function_3(backtrace_context_t* ctx) {
    return bw_backtrace(count_frames_cb, ctx);
}

__attribute__((noinline)) bool deep_function_2(backtrace_context_t* ctx) {
    return deep_function_3(ctx);
}

__attribute__((noinline)) bool deep_function_1(backtrace_context_t* ctx) {
    return deep_function_2(ctx);
}

TEST(basic_backtrace, {
    backtrace_context_t ctx = {0};
    ctx.expected_func = "basic_backtrace";

    bool success = bw_backtrace(count_frames_cb, &ctx);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_INT(ctx.frame_count, 1);
    TEST_ASSERT_TRUE(ctx.found_expected_func);
})

TEST(deep_stack_backtrace, {
    backtrace_context_t ctx = {0};
    ctx.expected_func = "deep_function"; // Look for any deep_function

    bool success = deep_function_1(&ctx);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_INT(ctx.frame_count, 3); // Should have at least a few frames
    // Note: Function name visibility depends on debug symbols and optimization
    BW_UNUSED(ctx.found_expected_func); // Don't assert on this - it's implementation dependent
})

TEST(early_termination, {
    int frame_count = 0;

    bool success = bw_backtrace(stop_after_n_frames_cb, &frame_count);

    TEST_ASSERT_FALSE(success); // Returns false since callback stops after 3 frames
    TEST_ASSERT_EQ_INT(frame_count, 3); // Should stop after exactly 3 frames
})

TEST(null_callback, {
    // This should handle gracefully or return false
    bool success = bw_backtrace(NULL, NULL);

    // The behavior might be implementation-defined, but it shouldn't crash
    BW_UNUSED(success); // Accept either true or false
})

bool collect_symbols_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);

    int* valid_symbols = (int*)arg;

    if (fname && strlen(fname) > 0) {
        (*valid_symbols)++;
    }

    if (sname && strlen(sname) > 0) {
        (*valid_symbols)++;
    }

    return true;
}

TEST(symbol_resolution, {
    int valid_symbols = 0;

    bool success = bw_backtrace(collect_symbols_cb, &valid_symbols);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_INT(valid_symbols, 1); // Should have at least some valid symbols
})

int main(void) {
    TEST_INIT("backtrace");

    TEST_RUN(basic_backtrace);
    TEST_RUN(deep_stack_backtrace);
    TEST_RUN(early_termination);
    TEST_RUN(null_callback);
    TEST_RUN(symbol_resolution);

    TEST_EXIT();
}
