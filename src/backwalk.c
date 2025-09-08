// NOLINTNEXTLINE(bugprone-reserved-identifier, readability-identifier-naming)
#define _GNU_SOURCE
#include "backwalk/backwalk.h"

#include <dlfcn.h>    // for dladdr, Dl_info
#include <stdbool.h>  // for bool, false, true
#include <stddef.h>   // for NULL
#include <stdint.h>   // for uintptr_t

#include "context.h"  // for context_get_ip, context_init, context_step, con...
#include "debug.h"    // for BW_PRINT_FRAME

bool bw_backtrace(bw_backtrace_cb cb, void* arg) {
    context_t ctx;
    context_init(&ctx);

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

        BW_PRINT_FRAME(mod_addr, fname, sname);

        if (cb && !cb(mod_addr, fname, sname, arg)) {
            return false;
        }
    }

    return true;
}
