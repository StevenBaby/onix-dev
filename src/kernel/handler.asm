[bits 64]
; 中断处理函数入口 

extern handler_table

section .text

%macro INTERRUPT_HANDLER 2
interrupt_handler_%1:
%ifn %2
    push ONIX_MAGIC
%endif
    push %1; 压入中断向量，跳转到中断入口
    jmp interrupt_entry
%endmacro

interrupt_entry:
    push rbp
    mov rbp, rsp

    push rax
    push rcx
    push rdx
    push rbx

    push rsi
    push rdi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, ds
    push rax

    mov rax, es
    push rax

    mov rax, fs
    push rax

    mov rax, gs
    push rax

    ; 找到前面 push %1 压入的 中断向量
    mov rdi, [rbp + 8]
    call [handler_table + rdi * 8]

global interrupt_exit
interrupt_exit:

    ; 恢复下文寄存器信息
    pop rax
    mov gs, rax

    pop rax
    mov fs, rax

    pop rax
    mov es, rax

    pop rax
    mov ds, rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rdi
    pop rsi

    pop rbx
    pop rdx
    pop rcx
    pop rax

    pop rbp

    ; 对应 push %1
    ; 对应 error code 或 push magic
    add rsp, 16

    iretq

INTERRUPT_HANDLER 0, 0; divide by zero
INTERRUPT_HANDLER 1, 0; debug
INTERRUPT_HANDLER 2, 0; non maskable interrupt
INTERRUPT_HANDLER 3, 0; breakpoint

INTERRUPT_HANDLER 4, 0; overflow
INTERRUPT_HANDLER 5, 0; bound range exceeded
INTERRUPT_HANDLER 6, 0; invalid opcode
INTERRUPT_HANDLER 7, 0; device not avilable

INTERRUPT_HANDLER 8, 1; double fault
INTERRUPT_HANDLER 9, 0; coprocessor segment overrun
INTERRUPT_HANDLER 10, 1; invalid TSS
INTERRUPT_HANDLER 11, 1; segment not present

INTERRUPT_HANDLER 12, 1; stack segment fault
INTERRUPT_HANDLER 13, 1; general protection fault
INTERRUPT_HANDLER 14, 1; page fault
INTERRUPT_HANDLER 15, 0; reserved

INTERRUPT_HANDLER 16, 0; x87 floating point exception
INTERRUPT_HANDLER 17, 1; alignment check
INTERRUPT_HANDLER 18, 0; machine check
INTERRUPT_HANDLER 19, 0; SIMD Floating - Point Exception

INTERRUPT_HANDLER 20, 0; Virtualization Exception
INTERRUPT_HANDLER 21, 1; Control Protection Exception
INTERRUPT_HANDLER 22, 0; reserved
INTERRUPT_HANDLER 23, 0; reserved

INTERRUPT_HANDLER 24, 0; reserved
INTERRUPT_HANDLER 25, 0; reserved
INTERRUPT_HANDLER 26, 0; reserved
INTERRUPT_HANDLER 27, 0; reserved

INTERRUPT_HANDLER 28, 0; reserved
INTERRUPT_HANDLER 29, 0; reserved
INTERRUPT_HANDLER 30, 0; reserved
INTERRUPT_HANDLER 31, 0; reserved

%assign i 32
%rep (256 - 32)
    INTERRUPT_HANDLER i, 0;
%assign i i+1
%endrep

; 下面的数组记录了每个中断入口函数的指针
section .data
global handler_entry_table
handler_entry_table:

%macro INTERRUPT_ENTRY 1
    dq interrupt_handler_%1
%endmacro

%assign i 0
%rep 256
    INTERRUPT_ENTRY i
%assign i i+1
%endrep
