#ifndef BW_CONTEXT_H
#define BW_CONTEXT_H

#include <stdbool.h>    // for bool
#include <stdint.h>     // for uintptr_t

#include "sanitizer.h"  // for MSAN_UNPOISON

enum { CONTEXT_DATA_LEN = 8 };

typedef struct {
    uintptr_t data[CONTEXT_DATA_LEN];
} context_t;

void context_init_arch(context_t* ctx);

static inline __attribute__((always_inline)) void context_init(context_t* ctx) {
    context_init_arch(ctx);

    MSAN_UNPOISON(ctx->data, sizeof(ctx->data));
}

bool context_step(context_t* ctx);

uintptr_t context_get_ip(const context_t* ctx);

#endif // BW_CONTEXT_H
