[bits 32]

magic   equ 0xe85250d6
i386    equ 0
length  equ header_end - header_start

section .multiboot2
header_start:
    dd magic  ; 魔数
    dd i386   ; 32位保护模式
    dd length ; 头部长度
    dd -(magic + i386 + length); 校验和

    ; 结束标记
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

code_selector equ (1 << 3)
data_selector equ (2 << 3)

extern setup_long_mode

section .text
global _start
_start:
    push ebx; ards_count
    push eax; magic
    call setup_long_mode
    jmp code_selector:long_mode

align 8
[bits 64]

extern kernel_init

long_mode:

    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax;

    mov rsp, 0x10000; update stack top

    call kernel_init
    jmp $
