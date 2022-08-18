[org 0x7c00]

mov [BOOT_DISK], dl
call read_disk

jmp PROGRAM_SPACE
%include "read_disk.asm"
%include "io.asm"
times 510 - ($ - $$) db 0
dw 0xaa55