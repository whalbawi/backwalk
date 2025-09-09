# Usage

Backwalk calls a user-provided callback for each frame as it walks the stack:

```c
typedef bool (*bw_backtrace_cb)(uintptr_t addr, const char* fname, const char* sname, void* arg);
```

**Callback parameters:**
- `addr`: Module-relative address of the frame  
- `fname`: Filename (executable/library), or `"?"` if unknown  
- `sname`: Symbol name (function name), or `"?"` if unknown
- `arg`: User context passed through from `bw_backtrace()` (can be `NULL`).

The `addr` parameter contains module-relative offsets, not absolute virtual addresses. This works
well with PIE (Position Independent Executable) binaries where modules are loaded at different base
addresses each run. For non-PIE executables (compiled with `-no-pie`), the main executable's
addresses will be reported relative to the executable's base load address, usually 0x400000 on x64
Linux. Consequently, the reported addresses will not match what is reported by e.g. `objdump` or
`nm`. 

Return `true` to continue walking, `false` to stop.

## Basic Usage
```c
bool print_frame(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    printf("0x%016lx: %s (%s)\n", addr, sname, fname);
    return true;  // Continue walking
}

bw_backtrace(print_frame, NULL);
```

The parameter `arg` can be used to maintain state:
## Collecting Frames

```c
typedef struct {
    uintptr_t* addrs;
    size_t count;
    size_t max_frames;
} collector_t;

bool collect_frame(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    collector_t* c = (collector_t*)arg;
    if (c->count < c->max_frames) {
        c->addrs[c->count++] = addr;
        return true;
    }
    return false;  // Stop when full
}

collector_t collector = {buffer, 0, 16};
bw_backtrace(collect_frame, &collector);
```

## C++ Usage

C++ is supported, but note that symbol names are not automatically demangled.

```cpp
#include <vector>
#include <backwalk/backwalk.h>

namespace stacktrace {

std::vector<uintptr_t> addresses;

bool collect_cpp(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    auto* vec = static_cast<std::vector<uintptr_t>*>(arg);
    vec->push_back(addr);
    return true;
}

} // namespace stacktrace

bw_backtrace(stacktrace::collect_cpp, &addresses);
```
