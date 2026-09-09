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

    cmp eax, 8
    je delete_char_buffer

    movzx ecx, byte [user_pos]
    cmp byte [user_pos], 63
    jae read_key
    mov [user_buffer + ecx], al
    mov [user_char], al
    add byte [user_pos], 1
    
    mov eax, 0
    mov ebx, user_char
    mov ecx, 1
    int 0x80

    jmp read_key


command_ready:

    call print_newline
    movzx ecx, byte [user_pos]
    mov byte [user_buffer + ecx], 0

    
    mov ecx, 0
    mov esi, user_buffer

    mov edi, help_command
    call compare_string
    cmp eax, 1
    je help_command_handl

    mov edi, clear_command
    call compare_string
    cmp eax, 1
    je clear_command_handl

    
    mov edi, exit_command
    call compare_string
    cmp eax, 1
    je exit_command_handl

    jmp no_command


    
    
clear_command_handl:
    call clear_command_handler
    jmp no_command
help_command_handl:
    call help_command_handler
    jmp no_command
exit_command_handl:
    call exit_command_handler
    jmp no_command
no_command:
    call clear_user_buffer
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

    mov al, [esi + ecx]
    mov bl, [edi + ecx]

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
    mov eax, 0
    mov ebx, help_message
    mov ecx, help_message_len
    int 0x80
    call print_newline
    ret

delete_char_buffer:
    cmp byte [user_pos], 0
    je read_key
    sub byte [user_pos], 1
    movzx ecx, byte[user_pos]
    mov byte [user_buffer + ecx], 0
    mov eax, 5
    int 0x80
    
    jmp read_key
exit_command_handler:
    mov eax, 1
    int 0x80
    ret

clear_command_handler:
    mov eax, 4
    int 0x80
    ret
hang:
    jmp hang

section .data

prompt:
    db "User>"
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
clear_command:
    db "clear", 0
exit_command:
    db "exit", 0
help_message:
    db "Commands:",10,"help  - Give command info",10,"clear  - Clear Terminal", 10, "exit  - Exit shell"
help_message_len equ $ - help_message
