.section .text
.global _start

_start:
    // Preserve bootinfo (in x0) and setup stack
    mov x1, x0
    ldr x2, =stack_top
    mov sp, x2
    // restore bootinfo into x0
    mov x0, x1

    bl kernel_main

1:  wfe
    b 1b

.section .bss
.align 12
stack:
    .skip 4096
stack_top: