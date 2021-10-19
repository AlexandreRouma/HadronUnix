drive_number: db 0
drive_sector_per_track: db 0
drive_head_count: db 0

; AL = Number of sectors
; CH = Cylinder
; CL = Sector
; DH = Head
; DL = Drive
; ES:BX = Buffer Pointer
drive_read_sectors:
    mov ah, 02h
    int 13h
    jc dump_drive_error
    ret

; DL = Drive
drive_update_info:
    ; Get drive info from BIOS
    mov ax, 0
    mov es, ax
    mov di, ax
    mov ah, 08h
    int 13h
    jc dump_drive_error

    ; Decode info
    inc dh
    and cx, 111111b
    mov [drive_head_count], dh
    mov [drive_sector_per_track], cl

    ret

; EAX = LBA
; Returns:
; CL = Sector
; DH = Head
; CH = Cylinder
drive_lba_to_chs:
    xor ebx, ebx
    mov bl, [drive_sector_per_track]
    xor edx, edx
    div ebx
    mov cl, dl
    inc cl
    xor ebx, ebx
    mov bl, [drive_head_count]
    xor edx, edx
    div ebx
    mov dh, dl
    mov ch, al
    ret