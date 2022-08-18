get_char:
    mov ah, 0x0
    int 0x16
    jmp .exit

    .exit:
        ret

print:
    mov ah, 0x0e

    .loop:
        mov al, [bx]
        cmp al, 0x0
        je .exit
        int 0x10
        inc bx
        jmp .loop

    .exit:
        mov al, 0x0
        ret

println:
    call print
    call newline
    jmp .exit

    .exit:
        ret

newline:
    mov ah, 0x0e
    mov al, 0x0d
    int 0x10
    mov al, 0xa
    int 0x10
    jmp .exit

    .exit:
        mov al, 0x0
        ret