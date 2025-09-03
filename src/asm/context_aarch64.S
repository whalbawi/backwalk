#if defined(__aarch64__)

.text
.globl context_init
.type context_init, %function
context_init:
    # Stack base for caller
    str  x29, [x0]

    # Where we're returning to
    ldr  x9, [x29]
    str  x9, [x0, #8]

    ret

#endif // defined(__aarch64__)
