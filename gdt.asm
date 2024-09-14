GDTNullDesc:
    dd 0, 0

GDTCodeSeg:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

GDTDataSeg:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

GDTDescriptor:
    GDTSize:
        dw GDTDescriptor - GDTNullDesc - 1
        dd GDTNullDesc

CodeSeg equ GDTCodeSeg - GDTNullDesc
DataSeg equ GDTDataSeg - GDTNullDesc

[bits 32]
EditGDT:
    mov [GDTCodeSeg + 6], byte 10101111b
    mov [GDTDataSeg + 6], byte 10101111b
    ret

[bits 16]