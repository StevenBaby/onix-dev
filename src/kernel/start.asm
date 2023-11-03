[bits 32]

extern i386_init

section .text
global _start
_start:
    call i386_init

    jmp $; block
