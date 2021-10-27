BITS 32

SECTION .data
ALIGN 4096
; Paging tables
longmode_pml4:
   times 4096 db 0
longmode_pml4_end:

longmode_pdpe:
    times 4096 db 0
longmode_pdpe_end:

longmode_pde:
    times 4096 db 0
longmode_pde_end:

; Long mode GDT
longmode_gdt:
    ; Null Entry
    dq 0

longmode_gdt_code_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10011010b
    db 10101111b
    db 00h

longmode_gdt_data_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10010010b
    db 10101111b
    db 00h

longmode_gdtr:
    dw longmode_gdtr - longmode_gdt - 1
    dq longmode_gdt

SECTION .text
GLOBAL longmode_call
longmode_call:
    ; Fill the first entry of the PML4 to point to the PDPE
    mov eax, longmode_pdpe
    or eax, 111b
    mov [longmode_pml4], eax
    mov [longmode_pml4+4], dword 0

    ; Fill the first entry of the PDPE to point to the PDE
    mov eax, longmode_pde
    or eax, 111b
    mov [longmode_pdpe], eax
    mov [longmode_pdpe+4], dword 0

    ; Fill the PDPE to 1:1 map 512GB
    mov ecx, longmode_pde
    xor edx, edx
longmode_call_pde_loop:
    ; Move lower 11 bits to the last 11 bits of eax
    mov eax, edx
    and eax, 11111111111b
    shl eax, 21

    ; Move upper 21 bits to the first 21 bits of ebx
    mov ebx, edx
    shr ebx, 11

    ; Add flags to eax
    or eax, 10000111b

    ; Write entry
    mov [ecx], eax
    mov [ecx+4], ebx

    ; Increment counters and loop if not done
    add ecx, 8
    inc edx
    cmp ecx, longmode_pde_end
    jb longmode_call_pde_loop

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
    lgdt [longmode_gdtr]
    jmp (longmode_gdt_code_entry - longmode_gdt):longmode_stub
    
BITS 64
longmode_stub:
    ; Load registers from the stack
    add rsp, 4
    pop rax
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop r8
    pop r9

    ; Jump to 64bit code
    jmp rax

longmode_end:
    jmp longmode_end