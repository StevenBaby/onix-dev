[org 0x1000]

dd 0x55aa; magic number for check error
kernel_size: dd 0; kernel size 

; there are real mode

; print loading
mov si, loading
call print

jmp $

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

loading:
    db "Loading Onix...", 10, 13, 0; \n\r


