; pic.asm
; Legacy PIC utility

global DisablePIC
section .text
DisablePIC:
    mov al, 0xff
    out 0xa1, al
    out 0x21, al
    ret
