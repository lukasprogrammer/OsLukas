[BITS 16]
[ORG 0x7C00]

STAGE2_LOAD_SEG equ 0x0800
STAGE2_LBA      equ 1

start:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    sti

    ; VGA 80x25 text mode
    mov ax, 0x0003
    int 0x10

    ; Check INT 13h extensions
    call check_lba

    ; Load stage 2
    call load_stage2

    ; Pass boot drive to stage 2 in DL
    mov dl, [boot_drive]

    ; stage2 was loaded at physical 0x8000
    jmp 0x0000:0x8000


check_lba:
    mov dl, [boot_drive]
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13

    jc disk_read_error

    cmp bx, 0xAA55
    jne disk_read_error

    test cx, 1
    jz disk_read_error

    ret


load_stage2:
    xor ax, ax
    mov ds, ax

    mov si, stage2_disk_packet
    mov dl, [boot_drive]

    mov ah, 0x42
    int 0x13

    jc disk_read_error
    ret


disk_read_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10

.hang:
    cli
    hlt
    jmp .hang


boot_drive:
    db 0


stage2_disk_packet:
    db 0x10
    db 0x00
    dw 1
    dw 0x0000
    dw STAGE2_LOAD_SEG
    dq STAGE2_LBA


times 510 - ($ - $$) db 0
dw 0xAA55