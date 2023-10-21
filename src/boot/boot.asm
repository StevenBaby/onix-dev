[org 0x7c00]

; set crt text mode, clear the screen
mov ax, 3
int 0x10

; init segment registers
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

; print booting message
mov si, booting
call print

; read disk
mov ah, 0x42
mov dl, 0x80
mov si, dap
int 0x13

; check read disk error
cmp ah, 0
jnz error

; check loader magic number
cmp word [0x1000], 0x55aa
jnz error

jmp 0:0x1008

; print in real mode
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

booting:
    db "Booting Onix...", 10, 13, 0; \n\r

error:
    mov si, .msg
    call print
    hlt; halt the CPU
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0

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

; fill remain byte with 0
times 510 - ($ - $$) db 0

; master boot sector last two byte must be 0x55 0xaa
; dw 0xaa55
db 0x55, 0xaa
