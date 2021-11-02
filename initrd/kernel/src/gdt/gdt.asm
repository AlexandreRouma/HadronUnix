BITS 64
SECTION .text

EXTERN k_gdtr

GLOBAL asm_gdt_load
asm_gdt_load:
    xchg bx, bx
    ; Load GDT
    lgdt [k_gdtr]

    ; Load data segments
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di

    ret