#if defined(__aarch64__)

.text
.globl context_init_arch
.type context_init_arch, %function
context_init_arch:
    # Stack base for caller
    str  x29, [x0]

    ret

#endif // defined(__aarch64__)
