align 0x1000

code_selector equ (1 << 3)
data_selector equ (2 << 3)

APIC_ID equ 0xFEE00020

[bits 16]
global startsmp
startsmp:
    ; xchg bx, bx

    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
 
    mov fs, ax
    mov gs, ax

    mov ax, 0
    mov ss, ax
    mov esp, 0x8000

    lgdt [gdt_ptr - startsmp]; load gdt

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; jmp to protected mode
    jmp dword code_selector:ap_protected_mode

; gdt32 definition

; memory start address, means all 4G address space
memory_base equ 0; 

; memory limit 4G / 4K - 1
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) - 1

gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base
gdt_base:
    dd 0, 0; NULL descriptor
gdt_code:
    dw memory_limit & 0xffff; limit 0 ~ 15 bits
    dw memory_base & 0xffff; base 0 ~ 15 bits
    db (memory_base >> 16) & 0xff; base 16 ~ 23 bits
    ; exists_ DPL 0 _ S _ code _ no conforming _ readable _ no access
    db 0b_1_00_1_1_0_1_0;
    ; 4k _ 32 bits _ no 64 bits _ avl _ limit 16 ~ 19
    db 0b_1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff; base 24 ~ 31 bits
gdt_data:
    dw memory_limit & 0xffff; limit 0 ~ 15 bits
    dw memory_base & 0xffff; base 0 ~ 15 bits
    db (memory_base >> 16) & 0xff; base 16 ~ 23 bits
    ; exists _ DPL 0 _ S _ data _ upward _ writeable _ no access
    db 0b_1_00_1_0_0_1_0;
    ; 4k _ 32 bits _ no 64 bits _ avl _ limit 16 ~ 19 bits
    db 0b_1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff; base 24 ~ 31 bits
gdt_end:

[bits 32]
extern setup_ap_long_mode
ap_protected_mode:
    ; xchg bx, bx; bochs magic breakpoint

    ; init segment registers
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax;

    ; set different stack with different ap
    mov eax, [APIC_ID];
    shr eax, 24
    shl eax, 12
    add eax, 0x10000
    mov esp, eax

    call setup_ap_long_mode
    jmp code_selector:startsmp_long_mode

align 8
[bits 64]
extern smp_ap_init
startsmp_long_mode:

    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax;

    call smp_ap_init

    hlt
    jmp $
