OUTPUT_FORMAT(binary)
ENTRY(Entry)

SECTIONS {
    . = 0x8000;
    .text : ALIGN(0x1000) { *(.text) }
    .idt BLOCK(0x1000) : ALIGN(0x1000) { IDT = .; . = . + 0x1000; }
    .data : ALIGN(0x1000) { *(.data) }
    .rodata : ALIGN(0x1000) { *(.rodata) }
    .bss : ALIGN(0x1000) { *(COMMON) *(.bss) }
}