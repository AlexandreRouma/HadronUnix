drive_number:
    db 0
drive_sector_per_track:
    db 0
drive_head_count:
    db 0

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
    and cl, 111111b
    inc dh
    mov [drive_head_count], dh
    mov [drive_sector_per_track], cl
    ret

; BL = Number of sectors
; EAX = LBA
; DL = Drive Number
; ES:SI = Buffer Pointer
drive_read_sectors:
    push dx

    ; Calculate TEMP and Sector - 1
    xor edx, edx
    xor ecx, ecx
    mov cl, [drive_sector_per_track]
    div ecx

    ; Save sector
    inc dl
    push dx

    ; Calculate Head and Cylinder
    xor edx, edx
    xor ecx, ecx
    mov cl, [drive_head_count]
    div ecx

    ; Stack: Sector, EAX: Cylinder, EDX: Head
    pop cx
    mov ch, al
    shr ax, 2
    and al, 0C0h
    or cl, al
    mov dh, dl

    ; Set the drive number, sector count and buffer
    pop ax
    mov dl, al
    mov al, bl
    mov bx, si

    ; Call BIOS
    mov ah, 02h
    int 13h

    jc dump_drive_error
    ret