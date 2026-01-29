.section .text
.global _start

_start:
    // Setup stack
    ldr x0, =stack_top
    mov sp, x0

    bl kernel_main

1:  wfe
    b 1b

.section .bss
.align 12
stack:
    .skip 4096
stack_top: