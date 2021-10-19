BITS 16
;ORG 7C00h
SECTION .boot

GLOBAL stage1_start
stage1_start:
    jmp start

%include "drive.asm"
%include "screen.asm"
%include "gdt.asm"

; In sectors
stage2_size: dd 0

start:
    ; Save boot data
    mov [drive_number], dl

    ; Set segment registers
    xor ax, ax
    mov cx, ax
    mov dx, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Prepare the stack
    mov sp, 7B00h
    mov bp, sp

    ; Get drive info
    call drive_update_info

    ; Read first data sector
    xor cx, cx
    mov es, cx
    mov al, 1
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, 7E00h
    mov dl, [drive_number]
    call drive_read_sectors

    ; Decode signature
    mov eax, [7E00h]
    cmp eax, 0DEADBEEFh
    jne inv_signature_error

    ; Decode size and convert to sectors
    mov eax, [7E04h]
    dec eax
    mov ebx, 512
    xor edx, edx
    div ebx
    inc eax
    mov [stage2_size], eax
    
    ; Load sectors (LBA is 1 because we read it right before)
    mov bx, 7E0h
    mov es, bx
    mov eax, 1
load_loop:
    ; Increment LBA
    inc eax
    mov bx, es
    add bx, 32
    mov es, bx

    ; Calculate CHS address
    push eax
    push es
    call drive_lba_to_chs

    ; Read sector
    mov al, 1
    mov dl, [drive_number]
    xor bx, bx
    pop es
    push es
    call drive_read_sectors
    pop es
    pop eax
    
    ; Check if done
    cmp eax, [stage2_size]
    jb load_loop

    ; Breakpoint
    ; xchg bx, bx

    ; Switch to protected mode
    cli
    lgdt [gdtr_data]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Set data segments
    mov ax, gdt_32bit_data_entry - gdt_data
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Long jump to stage2
    jmp (gdt_32bit_code_entry-gdt_data):7E08h

end:
    jmp end

inv_signature_error:
    mov bl, 0100b
    mov di, str_invalid_signature
    call print
    call advance_cursor_y
    jmp end

dump_drive_error:
    mov bl, 0100b
    mov di, str_drive_error
    call print
    call advance_cursor_y
    jmp end

%include "strings.asm"

; MBR
times 510-($-$$) db 0
db 0x55
db 0xAA