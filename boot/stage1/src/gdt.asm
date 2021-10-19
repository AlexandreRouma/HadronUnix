ALIGN 8

; Create dummy GDT
gdt_data:
    ; Null entry
    dq 0

gdt_32bit_code_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10011010b
    db 11001111b
    db 00h

gdt_32bit_data_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10010010b
    db 11001111b
    db 00h

gdt_16bit_code_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10011010b
    db 10001111b
    db 00h

gdt_16bit_data_entry:
    dw 0FFFFh
    dw 0000h
    db 00h
    db 10010010b
    db 10001111b
    db 00h

gdtr_data:
    dw gdtr_data - gdt_data - 1
    dd gdt_data