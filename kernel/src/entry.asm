BITS 64

SECTION .bss
GLOBAL kstack_bottom
GLOBAL kstack_top
kstack_bottom:
resb 65536
kstack_top:

SECTION .text
EXTERN kmain
GLOBAL kentry
kentry:
    ; Setup the stack
    mov rsp, kstack_top
    sub rsp, 8
    mov rbp, rsp

    ; Call kernel main
    call kmain