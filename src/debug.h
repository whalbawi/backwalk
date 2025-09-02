#ifndef BW_DEBUG_H
#define BW_DEBUG_H

#if !defined(BW_DEBUG_ENABLED)
#define BW_DEBUG_ENABLED 0
#endif

#if BW_DEBUG_ENABLED
#include <stdint.h>
#include <stdio.h>

#include "common.h"

void print_frame(int fnum, uintptr_t addr, const char* fname, const char* sname) {
    BW_UNUSED(fprintf(stderr, "#%2d [%#08jx] %s:%s\n", fnum, addr, fname, sname));
}

#endif // BW_DEBUG_ENABLED

#endif // BW_DEBUG_H
