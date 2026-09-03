[BITS 32]

global SwitchTask

SwitchTask:
    ;task A
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    push ebp
    push ebx
    push esi
    push edi

    mov [eax], esp


    ;actually switch task
    mov esp, edx

    ;task B
    pop edi
    pop esi
    pop ebx
    pop ebp

    ret

global RestoreTask

RestoreTask:
    mov eax, [esp + 4]
    mov esp, eax

    pop gs
    pop fs
    pop es
    pop ds

    popa
    add esp, 8

    iret