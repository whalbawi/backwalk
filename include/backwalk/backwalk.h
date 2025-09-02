#ifndef BW_BACKWALK_H
#define BW_BACKWALK_H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*bw_backtrace_cb)(uintptr_t addr, const char* fname, const char* sname, void* arg);

bool bw_backtrace(bw_backtrace_cb cb, void* arg);

#endif // BW_BACKWALK_H
