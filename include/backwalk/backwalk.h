#ifndef BW_BACKWALK_H
#define BW_BACKWALK_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdbool.h>  // for bool
#include <stdint.h>   // for uintptr_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*bw_backtrace_cb)(uintptr_t addr, const char* fname, const char* sname, void* arg);

bool bw_backtrace(bw_backtrace_cb cb, void* arg);

#ifdef __cplusplus
}
#endif

#endif // BW_BACKWALK_H
