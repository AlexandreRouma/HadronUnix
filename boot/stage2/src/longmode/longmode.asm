BITS 32

SECTION .data
ALIGN 4096
longmode_pml4:
   times 4096 db 0
longmode_pml4_end:

longmode_pdpe:
    times 4096 db 0
longmode_pdpe_end:

SECTION .text
GLOBAL longmode_call
longmode_call:
    xchg bx, bx
    ; Fill the first entry of the PML4 to point to the PDPE
    mov eax, longmode_pdpe
    or eax, 111b
    mov [longmode_pml4], eax
    mov [longmode_pml4+4], dword 0

    ; Fill the PDPE to 1:1 map 512GB
    mov ecx, longmode_pdpe
    xor edx, edx
longmode_call_pdpe_loop:
    ; Move lower 2 bits to the last 2 bits of edx
    mov eax, edx
    and eax, 11b
    shl eax, 30

    ; Move upper 30 bits to the first 20 bits of edi
    mov ebx, edx
    shr ebx, 2

    ; Add flags to eax
    or eax, 10000111b

    ; Write entry
    mov [ecx], eax
    mov [ecx+4], ebx

    ; Increment counters and loop if not done
    add ecx, 8
    inc edx
    cmp ecx, longmode_pdpe_end
    jb longmode_call_pdpe_loop

    ; Enable PAE
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; Enable LM bit in the EFER
    mov ecx, 0C0000080h
    rdmsr
    or eax, (1 << 8)
    wrmsr

    ; Enable paging (enters compatibility mode)
    mov eax, longmode_pml4
    mov cr3, eax
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; Enable 64bit sub-mode
