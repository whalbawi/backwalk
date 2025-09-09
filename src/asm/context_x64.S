#if defined(__x86_64__)

.text
.globl context_init
context_init:
    # Stack base for caller
    movq %rbp, (%rdi)

    ret

.section .note.GNU-stack,"",@progbits

#endif // defined(__x86_64__)
