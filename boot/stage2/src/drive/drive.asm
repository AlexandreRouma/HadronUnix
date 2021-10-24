SECTION .space_after_loader
ALIGN 16
GLOBAL drive_scratch_buffer
drive_scratch_buffer:
    times (512*127) db 0

BITS 16
SECTION .text

GLOBAL asm_drive_get_info
asm_drive_get_info:
    ; Call BIOS
    xor ax, ax
    mov es, ax
    mov di, ax
    mov ah, 08h
    int 13h
    jc _asm_drive_get_info_error

    ; Pack number of heads
    inc dh
    mov ah, dh

    ; Pack number of sectors
    and cl, 111111b
    mov al, cl
    
    retf

_asm_drive_get_info_error:
    mov al, ah
    and eax, 0FFh
    retf

GLOBAL asm_drive_read_sectors
asm_drive_read_sectors:
    ; Move high 16bits of BX into ES
    mov edi, ebx
    shr edi, 16
    mov es, di

    ; Call BIOS
    mov ah, 02h
    int 13h

    ; Return the error code
    shr ax, 8
    retf