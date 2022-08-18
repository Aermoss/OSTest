PROGRAM_SPACE equ 0x8000

read_disk:
    mov ah, 0x02
    mov bx, PROGRAM_SPACE
    mov al, 32
    mov dl, [BOOT_DISK]
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    jc raise_disk_read_error
    ret

raise_disk_read_error:
    mov bx, disk_read_error
    call print
    jmp $

disk_read_error:
    db "failed to read disk", 0x0d, 0xa, 0x0

BOOT_DISK:
    db 0