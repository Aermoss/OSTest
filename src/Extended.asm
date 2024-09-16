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
    call ActivateSSE
    call Entry
    jmp $

ActivateSSE:
    mov rax, cr0
    and ax, 0b11111101
    or ax, 0b00000001
    mov cr0, rax
    mov rax, cr4
    or ax, 0b1100000000
    mov cr4, rax
    ret

times 2048 - ($ - $$) db 0