[bits 32]

extern setup_long_mode

section .text
global _start
_start:
    call setup_long_mode

    jmp $; block
