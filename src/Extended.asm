jmp EnterProtectedMode
%include "GDT.asm"

EnterProtectedMode:
    call EnableA20
    cli
    lgdt [GDTDescriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CodeSeg:StartProtectedMode

EnableA20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

[bits 32]
%include "CPUId.asm"
%include "IdentityPaging.asm"

StartProtectedMode:
    mov ax, DataSeg
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    call DetectCPUId
    call DetectLongMode
    call SetupIdentityPaging
    call EditGDT
    jmp CodeSeg:StartLongMode

[bits 64]
[extern Entry]

%include "IDT.asm"

StartLongMode:
    call Entry
    jmp $

times 2048 - ($ - $$) db 0