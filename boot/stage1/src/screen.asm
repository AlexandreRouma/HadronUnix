; DS:DI String
; BL: Attribute
print:
    ; Load char and goto end if null
    mov al, [ds:di]
    cmp al, 0
    je _print_end

    ; Call int 10h with information
    mov ah, 09h
    xor bh, bh
    mov cx, 1
    int 10h

    ; Advance cursor
    call advance_cursor_x
    
    ; Increment the index and continue to next char
    inc di
    jmp print

_print_end:
    ret

; AL = Character
; BL = Attribute
putc:
    mov ah, 09h
    xor bh, bh
    mov cx, 1
    int 10h
    call advance_cursor_x
    ret

advance_cursor_x:
    ; Get cursor position
    mov ah, 03h
    xor bh, bh
    int 10h

    ; Increase X position
    inc dl

    ; Set new cursor position
    mov ah, 02h
    xor bh, bh
    int 10h

    ret

advance_cursor_y:
    ; Get cursor position
    mov ah, 03h
    xor bh, bh
    int 10h

    ; Increase Y position
    inc dh
    xor dl, dl

    ; Set new cursor position
    mov ah, 02h
    xor bh, bh
    int 10h

    ret