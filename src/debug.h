#ifndef BW_DEBUG_H
#define BW_DEBUG_H

#include <stdint.h>  // for uintptr_t

#if !defined(BW_DEBUG_ENABLED)
#define BW_DEBUG_ENABLED 0
#endif // BW_DEBUG_ENABLED

void print_frame(uintptr_t addr, const char* fname, const char* sname);

#if BW_DEBUG_ENABLED
#define BW_PRINT_FRAME(addr, fname, sname)                                                         \
    do {                                                                                           \
        print_frame(addr, fname, sname);                                                           \
    } while (0)
#else
#define BW_PRINT_FRAME(addr, fname, sname)

#endif // BW_DEBUG_ENABLED

#endif // BW_DEBUG_H
