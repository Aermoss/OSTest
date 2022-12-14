detect_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd

    pushfd
    pop eax
    push ecx
    popfd

    xor eax, ecx
    jz no_cpuid
    ret

detect_long_mode:
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz no_long_mode
    ret

no_cpuid:
    hlt

no_long_mode:
    hlt