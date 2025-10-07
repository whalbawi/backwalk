# Backwalk

**Backwalk** is a lightweight, cross-platform stack backtracing library written in C for x86_64, AArch64
and ARM architectures. It provides a simple callback-based interface for collecting stack traces
with symbol resolution.

## Features

- **Cross-platform**: Supports x86_64, AArch64, and ARM architectures
- **Frame pointer-based**: Uses frame pointer walking for stack traversal
- **Symbol resolution**: Automatic symbol resolution using `dladdr()`
- **Thread-safe**: Safe for use in multithreaded environments
- **C++ compatible**: Full C++ support with proper linkage

## Building

### Prerequisites

- **CMake** 3.20 or newer
- **GCC** 10+ or **Clang** (with C99 and C++11 support)
- **libdl** (typically included with glibc)
- **Frame pointers** must be enabled (`-fno-omit-frame-pointer`)

### Quick Build

```bash
git clone https://github.com/whalbawi/backwalk.git
cd backwalk
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run All Tests

```bash
cmake --build build
ctest --test-dir build -V
```

Individual test executables are available in the `build/` directory after building.


## Example

```c
#include <stdio.h>
#include <backwalk/backwalk.h>

bool print_frame(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    int* frame_index = (int*)arg;
    printf("[%d] 0x%012lx: %s (%s)\n", (*frame_index)++, addr, sname, fname);
    return true; // Continue walking
}

void deep_function() {
    printf("Stack trace:\n");
    int index = 0;
    bw_backtrace(print_frame, &index);
}

void middle_function() {
    deep_function();
}

int main() {
    middle_function();
    return 0;
}
```

**Sample output:**
```
Stack trace:
[0] 0x000000001234: deep_function (./example)
[1] 0x000000001278: middle_function (./example)
[2] 0x0000000012ab: main (./example)
[3] 0x000000029000: __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6)
```

Note: Addresses are module-relative offsets, not absolute virtual addresses. See [usage documentation](doc/usage.md#usage) for details on PIE vs non-PIE behavior.

**Compile and Link:**
```bash
gcc -fno-omit-frame-pointer -o example example.c -lbackwalk -ldl -rdynamic
```

**Important**: Always compile with `-fno-omit-frame-pointer` and link with `-rdynamic` for best
results.

For detailed usage patterns and complete examples, see the [`doc/`](doc/) directory.

### Limitations

- Requires frame pointers to be preserved (`-fno-omit-frame-pointer`)
- Currently supports only x86_64, AArch64, ARM architectures
- Symbol resolution limited by available symbol information
- NOT async-signal-safe (yet)

## License

MIT License - see [LICENSE](LICENSE) file for details.
