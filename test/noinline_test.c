#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "backwalk/backwalk.h"

#include "test.h"

#define MK_SNAME_EXP(ctx, i, fname)                                                                \
    do {                                                                                           \
        (ctx).sname_exp[i] = fname;                                                                \
        (ctx).sname_exp_len[i] = sizeof(fname);                                                    \
    } while (0)

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
    int fnum = ctx->fnum++;

    if (fnum < ctx->fnum_max && strcmp(sname, ctx->sname_exp[fnum]) != 0) {
        return false;
    }

    return true;
}

bool func_two(context_t* ctx) {
    return bw_backtrace(validate_backtrace, (void*)ctx);
}

bool func_one(context_t* ctx) {
    return func_two(ctx);
}

TEST(success) {
    context_t ctx;
    ctx.fnum = 0;
    ctx.fnum_max = 3;
    MK_SNAME_EXP(ctx, 0, "func_two");
    MK_SNAME_EXP(ctx, 1, "func_one");
    MK_SNAME_EXP(ctx, 2, "success");

    bool success = func_one(&ctx);
    TEST_ASSERT_GE_INT(ctx.fnum, ctx.fnum_max);
    TEST_ASSERT_TRUE(success);

    TEST_OK();
}

int main(void) {
    TEST_INIT("noinline");

    TEST_RUN(success);

    TEST_EXIT();
}
