#if defined(__arm__)

.text
.globl context_init_arch
.type context_init_arch, %function
context_init_arch:
    # Stack base for caller
    str  r11, [r0]
        
    mov  pc,  lr

#endif // defined(__arm__)
