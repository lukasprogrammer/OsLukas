[BITS 32]

global _start

_start:
    mov eax, 0
    mov ebx, message
    mov ecx, message_len

    int 0x80

    mov eax, 1
    int 0x80

.hang:
    jmp .hang

message:
    db "Hello from real user program!", 10

message_len equ $ - message