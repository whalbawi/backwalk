// NOLINTNEXTLINE(bugprone-reserved-identifier, readability-identifier-naming)
#define _GNU_SOURCE
#include "backwalk/backwalk.h"

#include <dlfcn.h>    // for dladdr, Dl_info
#include <stdbool.h>  // for bool, false, true
#include <stddef.h>   // for NULL, size_t
#include <stdint.h>   // for uintptr_t
#include <stdlib.h>   // for free, malloc

#include "context.h"  // for context_get_ip, context_init, context_step, con...
#include "debug.h"    // for BW_PRINT_FRAME

static bool bw_frame_process(uintptr_t ip, bw_backtrace_cb cb, void* arg) {
    Dl_info info;
    uintptr_t mod_addr = 0;
    const char* fname = NULL;
    const char* sname = NULL;

        // NOLINTNEXTLINE(performance-no-int-to-ptr)
    if (dladdr((const void*)(ip - 1), &info)) {
        mod_addr = ip - (uintptr_t)info.dli_fbase;
    }
    fname = info.dli_fname ? info.dli_fname : "?";
    sname = info.dli_sname ? info.dli_sname : "?";

    BW_PRINT_FRAME(mod_addr, fname, sname);

    if (cb && !cb(mod_addr, fname, sname, arg)) {
        return false;
    }

    return true;
}

bool bw_backtrace(bw_backtrace_cb cb, void* arg) {
    context_t ctx;
    context_init(&ctx);

    while (context_step(&ctx)) {
        uintptr_t ip = context_get_ip(&ctx);
        if (!bw_frame_process(ip, cb, arg)) {
            return false;
        };
    }

    return true;
}

struct bw_context {
    uintptr_t* ip;
    size_t ip_cnt;
    size_t ip_max_cnt;
};

bw_context_t* mkbw_context(size_t depth) {
    bw_context_t* bw_ctx = malloc(sizeof(bw_context_t));
    if (bw_ctx == NULL) {
        return NULL;
    }
    uintptr_t* ip = malloc(sizeof(uintptr_t) * depth);
    if (ip == NULL) {
        free(bw_ctx);
        return NULL;
    }

    bw_ctx->ip = ip;
    bw_ctx->ip_cnt = 0;
    bw_ctx->ip_max_cnt = depth;

    return bw_ctx;
}

void bw_context_fini(bw_context_t* bw_ctx) {
    if (bw_ctx == NULL) {
        return;
    }

    if (bw_ctx->ip != NULL) {
        free(bw_ctx->ip);
    }

    free(bw_ctx);
}

bool bw_backtrace_collect(bw_context_t* bw_ctx) {
    context_t ctx;
    context_init(&ctx);
    size_t ip_cnt = 0;

    while (context_step(&ctx)) {
        if (ip_cnt >= bw_ctx->ip_max_cnt) {
            return false;
        }
        uintptr_t ip = context_get_ip(&ctx);
        bw_ctx->ip[ip_cnt++] = ip;
        bw_ctx->ip_cnt = ip_cnt;
    }

    return true;
}

bool bw_backtrace_process(bw_context_t* bw_ctx, bw_backtrace_cb cb, void* arg) {
    for (size_t i = 0; i < bw_ctx->ip_cnt; ++i) {
        if (!bw_frame_process(bw_ctx->ip[i], cb, arg)) {
            return false;
        };
    }

    return true;
}
