[org 0x1000]

dd 0x55aa; magic number for check error
kernel_size: dd KERNEL_SIZE; kernel size 

; there are real mode

; print detecting memory
mov si, detecting
call print

detect_memory:
    ; set ebx to 0
    xor ebx, ebx

    ; es:di structure address
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150; fix signature

.next:
    ; function
    mov eax, 0xe820
    ; ards size
    mov ecx, 20
    ; invoke 0x15 system call
    int 0x15

    ; if CF set, there was a error
    jc error

    ; move address to next structure
    add di, cx

    ; increase ards count
    inc dword [ards_count]

    cmp ebx, 0
    jnz .next

; print loading message
mov si, loading
call print

; read kernel
read_kernel:

    sub esp, 4 * 3; three variable
    mov dword [esp], 0; read bytes
    mov dword [esp + 4], 10     ; ecx start sector
    mov dword [esp + 8], 0x10000; edi target memory
    BLOCK_SIZE equ 100          ; sector count for read block

.read_block: 

    mov ax, [esp + 8]    ; target memory offset
    mov [dap.offset], ax

    mov ax, [esp + 10]  ; target memory segment
    shl ax, 12          ; shift left address = (segment << 4 + offset)
    mov [dap.segment], ax

    mov eax, [esp + 4]     ; start sector
    mov dword [dap.lbal], eax

    mov ax, BLOCK_SIZE  ; sector count
    mov [dap.sectors], ax

    call read_disk

    add dword [esp + 8], BLOCK_SIZE * 512  ; edi update target memory
    add dword [esp + 4], BLOCK_SIZE        ; ecx update start sector

    mov eax, [kernel_size]
    add dword [esp], BLOCK_SIZE * 512
    cmp dword [esp], eax; check kernel size

    ; if read memory less than kernel size then read next block
    jl .read_block

; print preparing message
mov si, preparing
call print

prepare_protected_mode:
    ; xchg bx, bx; breakpoint

    cli; clear IF flag, disable interrupt

    ; open A20 line
    in al,  0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr]; load gdt

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; jmp to protected mode
    jmp dword code_selector:protected_mode

print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

read_disk:
    ; read disk
    mov ah, 0x42
    mov dl, 0x80
    mov si, dap
    int 0x13

    ; check error
    cmp ah, 0
    jnz error
    ret

detecting:
    db "Detecting Memory...", 10, 13, 0; \n\r
loading:
    db "Loading Onix...", 10, 13, 0; \n\r
preparing:
    db "Preparing Protected Mode...", 10, 13, 0; \n\r

error:
    mov si, .msg
    call print
    hlt; halt CPU
    jmp $
    .msg db "Loading Error!!!", 10, 13, 0

    align 4         ; aligned to 4 byte
dap:                ; Disk Address Packet
    .size db 0x10   ; DAP size 16 byte
    .unused db 0x00 ; reserved
    .sectors dw 8   ; sector count

    ; becase of little-endian，so offset:segment
    .offset dw 0x1000   ; offset
    .segment dw 0x00    ; segment register
    .lbal dd 0x02       ; lba low 32 bits
    .lbah dd 0x00       ; lba high 16 bits

; there are 32 bits code

align 4
[bits 32]
protected_mode:
    ; xchg bx, bx; bochs magic breakpoint

    ; init segment registers
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax; 

    mov esp, 0x10000; update stack top

    mov eax, ONIX_MAGIC
    mov ebx, ards_count

    ; jmp to kernel
    jmp code_selector:ENTRYPOINT

    ud2; no way to there, error

code_selector equ (1 << 3)
data_selector equ (2 << 3)

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

; address range descriptor structrue buffer
ards_count:
    dd 0
ards_buffer:
