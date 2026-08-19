[BITS 32]
global _start

extern kernel_main



_start:

    mov word [0xB8004], 0x074B    ; K
    call kernel_main
    
    jmp $


times 512-($-$$) db 0