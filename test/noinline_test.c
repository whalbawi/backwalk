#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "test_common.h"
#include "backwalk/backwalk.h"

typedef struct {
    int fnum;
    int fnum_max;
    const char* sname_exp[3];
    size_t sname_exp_len[3];
} context_t;

bool validate_backtrace(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);

    context_t* ctx = arg;
    int fnum = ctx->fnum;

    if (fnum < ctx->fnum_max && strcmp(sname, ctx->sname_exp[fnum]) != 0) {
        return false;
    }

    ++ctx->fnum;

    return true;
}

#define MK_SNAME_EXP(ctx, i, fname)                                                                \
    do {                                                                                           \
        (ctx).sname_exp[i] = fname;                                                                \
        (ctx).sname_exp_len[i] = sizeof(fname);                                                    \
    } while (0)

bool func_two(void) {
    context_t ctx;
    ctx.fnum = 0;
    ctx.fnum_max = 3;
    MK_SNAME_EXP(ctx, 0, "func_two");
    MK_SNAME_EXP(ctx, 1, "func_one");
    MK_SNAME_EXP(ctx, 2, "noinline_test");

    bool success = bw_backtrace(validate_backtrace, (void*)&ctx);

    return ctx.fnum > 0 && success;
}

bool func_one(void) {
    return func_two();
}

test_result_t noinline_test(void) {
    return func_one() ? TEST_RESULT_OK : TEST_RESULT_FAIL;
}

#define TEST_RESULT_UPDATE(final, result)                                                          \
    do {                                                                                           \
        if ((result) != TEST_RESULT_OK) {                                                          \
            (final) = (result);                                                                    \
        }                                                                                          \
    } while (0)

#define RUN_TEST(final_result, test_function)                                                      \
    do {                                                                                           \
        test_result_t test_result = (test_function)();                                             \
        TEST_RESULT_UPDATE(final_result, test_result);                                             \
        const char* result_str = NULL;                                                             \
        switch (test_result) {                                                                     \
        case TEST_RESULT_OK:                                                                       \
            result_str = "OK";                                                                     \
            break;                                                                                 \
        case TEST_RESULT_FAIL:                                                                     \
            result_str = "FAIL";                                                                   \
            break;                                                                                 \
        case TEST_RESULT_INVALID:                                                                  \
            result_str = "INVALID";                                                                \
            break;                                                                                 \
        default:                                                                                   \
            result_str = "UNKNOWN";                                                                \
        }                                                                                          \
        BW_UNUSED(fprintf(stderr, #test_function ": %s\n", result_str));                           \
    } while (0)

int main(void) {
    test_result_t final_result = TEST_RESULT_OK;
    RUN_TEST(final_result, noinline_test);

    return final_result;
}
