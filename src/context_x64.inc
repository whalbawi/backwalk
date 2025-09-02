#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "context.h"

bool context_step(context_t* ctx) {
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    uintptr_t* base = (uintptr_t*)ctx->data[0];

    if (base == NULL) {
        return false;
    }

    if (*base == ctx->data[0]) {
        return false;
    }

    ctx->data[0] = *base;
    ctx->data[1] = *(base + 1);

    return true;
}

uintptr_t context_get_ip(context_t* ctx) {
    return ctx->data[1];
}
