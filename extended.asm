jmp enter_protected_mode
%include "gdt.asm"

enter_protected_mode:
    call enable_a20
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp codeseg:start_protected_mode

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

[bits 32]
%include "cpuid.asm"
%include "identity_paging.asm"

start_protected_mode:
    mov ax, dataseg
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    call detect_cpuid
    call detect_long_mode
    call setup_identity_paging
    call edit_gdt
    jmp codeseg:start_long_mode

[bits 64]
[extern _start]

start_long_mode:
    mov edi, 0xb8000
    mov rax, 0x0f200f200f200f20
    mov ecx, 500
    rep stosq
    call _start
    jmp $

times 2048 - ($ - $$) db 0