[BITS 32]

global _start

section .text

_start:
    mov eax, 3
    int 0x80

    mov eax, 0            
    mov ebx, prompt
    mov ecx, prompt_len
    int 0x80

read_key:
    


    mov eax, 2
    int 0x80;

    test eax, eax
    jz read_key

    cmp eax, 10
    je command_ready

    movzx ecx, byte [user_pos]
    mov [user_buffer + ecx], al
    mov [user_char], al
    add byte [user_pos], 1
    
    mov eax, 0
    mov ebx, user_char
    mov ecx, 1
    int 0x80

    jmp read_key


command_ready:

    mov eax, 0
    mov ebx, newline
    mov ecx, 1
    int 0x80

    movzx ecx, byte [user_pos]
    mov byte [user_buffer + ecx], 0

    mov eax, 0
    mov ebx, user_buffer
    int 0x80
    
    mov ecx, 0
    call compare_string

    cmp eax, 0
    je no_command
    call help_command_handler

no_command:
    call clear_user_buffer
    call print_newline
    call print_prompt
    jmp read_key


clear_user_buffer:
    movzx ecx, byte [user_pos]
    mov byte [user_buffer + ecx], 0
    cmp ecx, 0
    je clear_done
    sub byte[user_pos], 1
    jmp clear_user_buffer

print_newline:
    mov eax, 0
    mov ebx, newline    
    mov ecx, 1
    int 0x80
    ret

print_prompt:
    mov eax, 0            
    mov ebx, prompt
    mov ecx, prompt_len
    int 0x80
    ret

compare_string:

    mov al, [user_buffer + ecx]
    mov bl, [help_command + ecx]

    cmp al, bl
    jne not_equal

    cmp al, 0
    je equal

    
    
    

    add ecx, 1
    jmp compare_string
    
equal:
    mov eax, 1
    ret
not_equal:
    mov eax, 0
    ret
clear_done:
    ret

help_command_handler:
    call print_newline
    mov eax, 0
    mov ebx, help_message
    mov ecx, help_message_len
    int 0x80
    ret



hang:
    jmp hang

section .data

prompt:
    db "user>"
prompt_len equ $ - prompt


user_buffer:
    times 64 db 0

user_char:
    db 0
user_pos:
    db 0
newline:
    db 10
help_command:
    db "help", 0
help_message:
    db "Hi, How can i help you today"
help_message_len equ $ - help_message
