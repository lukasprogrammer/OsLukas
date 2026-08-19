[BITS 16]
[ORG 0x7c00]


CODE_OFFSET equ 0x8
DATA_OFFSET equ 0x10


KERNEL_LOAD_SEG equ 0X1000

KERNEL_START_ADDR equ 0x10000

start:
    cli ;disable interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; enable interrupt

    mov ax, 0x0003
    int 0x10

;Load kernel

mov ax, KERNEL_LOAD_SEG
mov es, ax
xor bx, bx

mov dh, 0x00
mov dl, 0x80

mov cl, 0x02
mov ch, 0x00
mov ah, 0x02
mov al, 16

int 0x13

jc disk_read_error




load_PM:


    cli
    xor ax, ax
    mov ds, ax

    lgdt[gdt_descriptor]



    xor ax, ax
    mov ds, ax

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_OFFSET:PModeMain


disk_read_error:
    hlt

gdt_start:
    dd 0x0
    dd 0x0
    
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start -1 
    dd gdt_start


[BITS 32]

PModeMain:
    

    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov ebp, 0x9C00
    mov esp, ebp


    ;in al, 0x92
    ;or al, 2
    ;out 0x92, al

    jmp CODE_OFFSET:KERNEL_START_ADDR



times 510 - ($ - $$) db 0


dw 0xAA55