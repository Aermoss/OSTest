[extern IDT]
[extern ISRHandler]

global ISR, LoadIDT

IDTDescriptor:
    dw 4095
    dq IDT

ISR:
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    call ISRHandler
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax
    iretq

LoadIDT:
    lidt [IDTDescriptor]
    sti
    ret