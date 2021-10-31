BITS 16
SECTION .realmode_caller
ALIGN 16
GLOBAL asm_mmap_buf
asm_mmap_buf:
    times 32 db 0

SECTION .text
GLOBAL asm_mmap_get
asm_mmap_get:
    ; Load ES:DI
    mov ecx, eax
    and eax, 0FFFFh
    shr ecx, 16
    mov di, ax
    mov es, cx

    ; Load EAX and ECX with the call numbers and call BIOS
    mov eax, 0E820h
    mov ecx, 24
    int 15h

    ; Check if success and return accordingly
    jc asm_mmap_get_end
    mov eax, ebx
    retf
asm_mmap_get_end:
    mov eax, 0FFFFFFFFh
    retf