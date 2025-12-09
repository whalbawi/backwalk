#ifndef BW_BACKWALK_H
#define BW_BACKWALK_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>  // for bool
#include <stddef.h>   // for size_t
#include <stdint.h>   // for uintptr_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*bw_backtrace_cb)(uintptr_t addr, const char* fname, const char* sname, void* arg);

typedef struct bw_context bw_context_t;

bool bw_backtrace(bw_backtrace_cb cb, void* arg);

bw_context_t* mkbw_context(size_t depth);

void bw_context_fini(bw_context_t* ctx);

bool bw_backtrace_collect(bw_context_t* ctx);

bool bw_backtrace_process(bw_context_t* bw_ctx, bw_backtrace_cb cb, void* arg);

#ifdef __cplusplus
}
#endif

#endif // BW_BACKWALK_H
