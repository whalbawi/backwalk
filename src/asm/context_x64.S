#if defined(__x86_64__)

.text
.globl context_init
context_init:
    # Stack base for caller
    movq %rbp, %r9
    movq %r9, (%rdi)

    # Where we're returning to
    movq (%rbp), %r9
    movq %r9, 8(%rdi)

    ret

.section .note.GNU-stack,"",@progbits

#endif // defined(__x86_64__)
