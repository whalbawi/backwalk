
#if defined(__x86_64__) || defined(__aarch64__)
#include "context.h"

#include <stdbool.h>  // for false, bool, true
#include <stdint.h>   // for uintptr_t

enum {
    BW_MIN_MMAP_ADDR = 64 << 10, // Default on Linux 6.14 x86_64 - Ubuntu 24.04
};

#if defined(__x86_64__)
// Upper bound for canonical userspace addresses (48-bit x86-64)
static const uintptr_t BW_MAX_USER_ADDR = 0x00007FFFFFFFFFFFULL;
#endif

bool context_step(context_t* ctx) {
    if (!ctx) {
        return false;
    }

    if (ctx->data[0] < BW_MIN_MMAP_ADDR) {
        return false;
    }

#if defined(__x86_64__)
    // Check upper bound for canonical userspace addresses
    if (ctx->data[0] > BW_MAX_USER_ADDR) {
        return false;
    }
#endif

    // Check pointer alignment
    if (ctx->data[0] & (sizeof(uintptr_t) - 1)) {
        return false;
    }

    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    uintptr_t* base = (uintptr_t*)ctx->data[0];
    if (*base == ctx->data[0]) {
        return false;
    }

    ctx->data[0] = *base;
    ctx->data[1] = *(base + 1);

    return true;
}

uintptr_t context_get_ip(const context_t* ctx) {
    if (!ctx) {
        return 0;
    }

    return ctx->data[1];
}

#else
#error "unsupported platform: only x86_64 and aarch64 are supported"
#endif
