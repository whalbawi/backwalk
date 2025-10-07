#include <stdbool.h>            // for bool, true, false
#include <stddef.h>             // for size_t, NULL
#include <stdint.h>             // for uintptr_t
#include <string.h>             // for strlen, strcmp

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace

#include "test.h"               // for TEST, TEST_ASSERT_TRUE, TEST_RUN, TES...

enum { SNAME_ENTRIES_MAX = 16 };

#define MK_SNAME_EXP(ctx, i, fname)                                                                \
    do {                                                                                           \
        (ctx)->sname_exp[i] = fname;                                                               \
        (ctx)->sname_exp_len[i] = sizeof(fname);                                                   \
    } while (0)

typedef struct {
    size_t fnum;
    size_t sname_entries_len;
    const char* sname_exp[SNAME_ENTRIES_MAX];
    size_t sname_exp_len[SNAME_ENTRIES_MAX];
    bool sname_found[SNAME_ENTRIES_MAX];
} context_t;

bool validate_backtrace(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);

    context_t* ctx = arg;
    size_t fnum = ctx->fnum++;

    if (fnum < ctx->sname_entries_len && strcmp(sname, ctx->sname_exp[fnum]) == 0) {
        ctx->sname_found[fnum] = true;
    }

    return true;
}

typedef struct {
    size_t fnum;
    size_t fnum_max;
} stop_context_t;

bool stop_after_n_frames_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);
    
    stop_context_t* ctx = arg;
    ++ctx->fnum;
    if (ctx->fnum == ctx->fnum_max) {
        return false;
    }

    return true;
}

__attribute__((noinline)) bool deep_function_3(context_t* ctx) {
    return bw_backtrace(validate_backtrace, ctx);
}

__attribute__((noinline)) bool deep_function_2(context_t* ctx) {
    return deep_function_3(ctx);
}

__attribute__((noinline)) bool deep_function_1(context_t* ctx) {
    return deep_function_2(ctx);
}

TEST(basic_backtrace, {
    context_t ctx = {0};
    MK_SNAME_EXP(&ctx, 0, "basic_backtrace");
    ctx.sname_entries_len = 1;

    bool success = bw_backtrace(validate_backtrace, &ctx);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_SIZE(ctx.fnum, 3UL);
    TEST_ASSERT_TRUE(ctx.sname_found[0]);
})

TEST(deep_stack_backtrace, {
    context_t ctx = {0};

    MK_SNAME_EXP(&ctx, 0, "deep_function_3");
    MK_SNAME_EXP(&ctx, 1, "deep_function_2");
    MK_SNAME_EXP(&ctx, 2, "deep_function_1");
    ctx.sname_entries_len = 3;

    bool success = deep_function_1(&ctx);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GE_SIZE(ctx.fnum, ctx.sname_entries_len);
    for (size_t i = 0; i < ctx.sname_entries_len; ++i) {
        TEST_ASSERT_TRUE(ctx.sname_found[i]);
    }
})

TEST(early_termination, {
    stop_context_t ctx = {0};
    ctx.fnum_max = 3;

    bool success = bw_backtrace(stop_after_n_frames_cb, &ctx);

    TEST_ASSERT_FALSE(success); // Returns false since callback stops after 3 frames
    TEST_ASSERT_EQ_SIZE(ctx.fnum, ctx.fnum_max); // Should stop after exactly 3 frames
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
    TEST_ASSERT_GE_INT32(valid_symbols, 1); // Should have at least some valid symbols
})

int main(int argc, char** argv) {
    TEST_INIT("backtrace", argc, argv);

    TEST_RUN(basic_backtrace);
    TEST_RUN(deep_stack_backtrace);
    TEST_RUN(early_termination);
    TEST_RUN(null_callback);
    TEST_RUN(symbol_resolution);

    TEST_EXIT();
}
