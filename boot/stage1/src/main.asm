BITS 16
ORG 7C00h

stage1_start:
    ; Enforce CS:IP
    jmp 0:start

%include "drive.asm"
%include "screen.asm"
%include "gdt.asm"

; In sectors
stage2_size: dw 0

start:
    ; Save boot data
    mov [drive_number], dl

    ; Set segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Prepare the stack
    mov sp, 7B00h
    mov bp, sp

    ; Get information about the structure of the drivfe
    call drive_update_info

    ; Read first data sector
    mov bl, 1
    xor eax, eax
    inc eax
    mov dl, [drive_number]
    mov si, 7E00h
    xor cx, cx
    mov es, cx
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
    mov [stage2_size], ax
    
    ; Load following sectors
    mov cx, 7E0h
    xor eax, eax
    inc al
load_loop:
    inc eax
    add cx, 32

    mov bl, 1
    mov dl, [drive_number]
    mov es, cx
    xor si, si

    push eax
    push cx
    call drive_read_sectors
    pop cx
    pop eax

    cmp ax, [stage2_size]
    jb load_loop

    ; Save boot drive for stage2
    mov dl, [drive_number]

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