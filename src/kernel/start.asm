[bits 32]

section .text
global _start
_start:
    mov byte [0xb8000], 'K'

    jmp $; block
