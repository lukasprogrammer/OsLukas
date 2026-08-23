[BITS 32]

global _start
global switch_stack

extern kernel_main

_start:


    call kernel_main

    jmp $

switch_stack:
    mov eax, [esp + 4]    ; new_stack
    mov edx, [esp + 8]    ; entry function

    mov esp, eax
    mov ebp, eax

    call edx

.hang:
    cli
    hlt
    jmp .hang