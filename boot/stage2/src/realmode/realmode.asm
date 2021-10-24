ALIGN 8

asm_realmode_saved_gdtr:
    dq 0

asm_realmode_saved_idtr:
    dq 0

; Create dummy GDT
asm_realmode_gdt_data:
    ; Null entry
    dq 0

asm_realmode_gdt_16bit_code_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10011010b
    db 10001111b
    db 00h

asm_realmode_gdt_16bit_data_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10010010b
    db 10001111b
    db 00h

asm_realmode_gdtr_data:
    dw asm_realmode_gdtr_data - asm_realmode_gdt_data - 1
    dd asm_realmode_gdt_data

asm_realmode_bios_idtr:
    dw 3FFh
    dd 0

GLOBAL asm_realmode_enter
asm_realmode_enter:
    ; Save ebx, esi, edi, ebp, esp (+ 20 bytes on the stack)
    push ebx
    push esi
    push edi
    push ebp
    push esp

    ; Disable interrupts and save their state
    pushfd
    cli

    ; Save segment registers
    mov cx, ds
    push cx
    mov cx, es
    push cx
    mov cx, fs
    push cx
    mov cx, gs
    push cx
    mov cx, ss
    push cx
    push word 0
    
    ; Save current GDT and IDT
    sgdt [asm_realmode_saved_gdtr]
    sidt [asm_realmode_saved_idtr]

    ; Load 16bit GDT
    lgdt [asm_realmode_gdtr_data]

    ; Switch to 16bit protected mode
    jmp (asm_realmode_gdt_16bit_code_entry-asm_realmode_gdt_data):asm_realmode_enter16protected

BITS 16
asm_realmode_enter16protected:
    ; Load segments with a data entry in the 16bit GDT
    mov ax, (asm_realmode_gdt_16bit_data_entry-asm_realmode_gdt_data)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Load BIOS IVT
    lidt [asm_realmode_bios_idtr]

    ; Disable protected mode
    mov eax, cr0
    and eax, ~1
    mov cr0, eax

    ; Switch to 16bit real mode
    jmp 0:asm_realmode_enter16real

SECTION .realmode_caller
asm_realmode_enter16real:
    ; Reload 16bit segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Re enable interrupts (TODO: ONLY IF IT WAS ENABLED BEFORE)
    sti

    ; Load cs:ip
    mov esi, [esp + 40]
    mov edi, [esp + 44]

    ; Load registers with the given arguments
    mov eax, [esp + 48]
    mov ebx, [esp + 52]
    mov ecx, [esp + 56]
    mov edx, [esp + 60]

    ; Push CS:IP for the returning (0 for CS since per the links script we should be under 0xFFFF)
    push word 0
    push word asm_realmode_exit
    
    ; Call function to execute in real mode (return value in eax)
    push si
    push di
    retf

asm_realmode_exit:
    ; Disable interrupts (TODO: save if they are enabled or not)
    cli

    ; Load 32bit GDT
    lgdt [asm_realmode_saved_gdtr]

    ; Enable protected mode
    mov ecx, cr0
    or ecx, 1
    mov cr0, ecx

    ; Restore segment registers
    pop cx
    pop cx
    mov ss, cx
    pop cx
    mov gs, cx
    pop cx
    mov fs, cx
    pop cx
    mov es, cx
    pop cx
    mov ds, cx
    
    ; Switch to protected mode (TODO: use saved segment, don't hardcode)
    jmp 08h:asm_realmode_exit32bit

BITS 32
asm_realmode_exit32bit:
    ; Restore protected mode IDT
    lidt [asm_realmode_saved_idtr]

    ; Re-enabled interrupts if they were enabled
    pop ebx
    and ebx, (1 << 9)
    cmp ebx, 0
    je asm_realmode_exit_end
    sti

asm_realmode_exit_end:
    ; Restore ebx, esi, edi, ebp, esp
    pop esp
    pop ebp
    pop edi
    pop esi
    pop ebx

    ; Return to the normal code
    ret