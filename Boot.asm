[org 0x7c00]

call ReadDisk
jmp ProgramSpace

%include "ReadDisk.asm"
%include "IO.asm"

times 510 - ($ - $$) db 0
dw 0xaa55