BITS 16
SECTION .text
GLOBAL asm_vbe_call
asm_vbe_call:
    mov edi, ebx
    shr edi, 16
    mov es, di
    mov edi, ebx
    int 10h
    retf