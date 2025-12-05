#ifndef BW_SANITIZER_H
#define BW_SANITIZER_H

#include "common.h"

#ifdef __has_feature
#if __has_feature(memory_sanitizer)
#define BW_MSAN_ENABLED
#endif
#endif

#ifdef BW_MSAN_ENABLED
#include <sanitizer/msan_interface.h>
#endif

#ifdef BW_MSAN_ENABLED
#define MSAN_UNPOISON(addr, size) __msan_unpoison(addr, size)
#else
#define MSAN_UNPOISON(addr, size)                                                                  \
    {                                                                                              \
        BW_UNUSED(addr);                                                                           \
        BW_UNUSED(size);                                                                           \
    }

#endif

#endif // BW_SANITIZER_H
