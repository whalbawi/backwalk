# Backwalk
Backwalk is a C library for collecting stacktraces.

## Prerequisites
The project is built using [CMake](https://cmake.org/) and requires version 3.20 or newer to work correctly.

## Building
Backwalk is built and tested on Ubuntu 24.04. Both x86-64 and arm64
architectures are supported.

The following will generate the build system and then build all targets:
```bash
$ cd <path-to-backwalk>
$ cmake -B build
$ cmake --build build
```

## Tests
Start by building the unit tests executable:
```bash
$ cmake --build build --target backwalk-tests
```

Now run the unit tests using CMake's `ctest` utility:
```bash
$ ctest --test-dir build -R unit-tests -V
```

Alternatively, run the tests executable directly:
```bash
$ ./build/backwalk-tests
```
