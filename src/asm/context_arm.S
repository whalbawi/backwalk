#if defined(__arm__)

.text
.globl context_init
.type context_init, %function
context_init:
    # Stack base for caller
    str  r11, [r0]
    mov  pc,  lr

#endif // defined(__arm__)
