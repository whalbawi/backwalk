#if defined(__x86_64__)
#include "context_x64.inc" // IWYU pragma: keep
#elif defined(__aarch64__)
#include "context_aarch64.inc" // IWYU pragma: keep
#else
#error "unsupported platform"
#endif // defined(__x86_64__)
