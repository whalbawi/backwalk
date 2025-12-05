#if defined(__x86_64__)

.text
.globl context_init_arch
context_init_arch:
    # Stack base for caller
    movq %rbp, (%rdi)

    ret

.section .note.GNU-stack,"",@progbits

#endif // defined(__x86_64__)
