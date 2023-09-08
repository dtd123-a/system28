; cr3.asm
; Handles loading pagetables

bits 64
global LoadCR3
LoadCR3:
    mov cr3, rdi
    ret