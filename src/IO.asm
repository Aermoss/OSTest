GetChar:
    mov ah, 0x0
    int 0x16
    ret

WriteString:
    mov ah, 0x0e

    .Loop:
        mov al, [bx]
        cmp al, 0x0
        je .Exit
        int 0x10
        inc bx
        jmp .Loop

    .Exit:
        ret