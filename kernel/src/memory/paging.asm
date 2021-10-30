BITS 64
GLOBAL asm_paging_load_cr3
asm_paging_load_cr3:
    mov cr3, rdi
    ret