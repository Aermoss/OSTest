%macro IncBin 2
section .rodata
global %1, %1_Size

%1:
    incbin %2
    db 0

%1_Size:
    dq %1_Size - %1
%endmacro

IncBin Text, "../text.txt"