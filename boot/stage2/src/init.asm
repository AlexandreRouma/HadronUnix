BITS 32

; Boot header for stage1 to know how to load stage2
SECTION .header
GLOBAL stage2_header
stage2_header:
    dd 0DEADBEEFh
    dd 0
    jmp stage2_start

; First thing to be called
SECTION .text
EXTERN stage2_main
GLOBAL stage2_start
stage2_start:
    ; Setup the stack
    mov esp, 7B00h
    mov ebp, esp

    ; Call bootloader main
    and edx, 0FFh
    push edx
    call stage2_main

; Catch if stage2_main returns
stage2_hlt:
    hlt
    jmp stage2_hlt

SECTION .size_detect
GLOBAL stage2_size_detect
stage2_size_detect: