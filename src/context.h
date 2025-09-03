#ifndef BW_CONTEXT_H
#define BW_CONTEXT_H

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uintptr_t

enum { CONTEXT_DATA_LEN = 8 };

typedef struct {
    uintptr_t data[CONTEXT_DATA_LEN];
} context_t;

void context_init(context_t* ctx);

bool context_step(context_t* ctx);

uintptr_t context_get_ip(context_t* ctx);

#endif // BW_CONTEXT_H
