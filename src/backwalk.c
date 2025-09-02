// NOLINTNEXTLINE(bugprone-reserved-identifier, readability-identifier-naming)
#define _GNU_SOURCE
#include "backwalk/backwalk.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>

#include "context.h"
#include "debug.h"

bool bw_backtrace(bw_backtrace_cb cb, void* arg) {
    context_t ctx;
    context_init(&ctx);

#if BW_DEBUG_ENABLED
    int fnum = 0;
#endif

    while (context_step(&ctx)) {
        uintptr_t ip = context_get_ip(&ctx);
        Dl_info info;
        uintptr_t mod_addr = 0;
        const char* fname = NULL;
        const char* sname = NULL;

        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        if (dladdr((const void*)(ip - 1), &info)) {
            mod_addr = ip - (uintptr_t)info.dli_fbase;
        }
        fname = info.dli_fname ? info.dli_fname : "?";
        sname = info.dli_sname ? info.dli_sname : "?";

#if BW_DEBUG_ENABLED
        print_frame(fnum, mod_addr, fname, sname);
        ++fnum;
#endif
        if (!cb(mod_addr, fname, sname, arg)) {
            return false;
        }
    }

    return true;
}
