ProgramSpace equ 0x8000

ReadDisk:
    mov ah, 0x02
    mov bx, ProgramSpace
    mov al, 32
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    jc .RaiseError
    ret

    .RaiseError:
        mov bx, DiskReadError
        call WriteString
        jmp $

DiskReadError:
    db "Failed to read disk.", 0x0d, 0xa, 0x0