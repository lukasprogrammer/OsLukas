[BITS 16]
[ORG 0x8000]

CODE_OFFSET       equ 0x08
DATA_OFFSET       equ 0x10

KERNEL_LOAD_SEG   equ 0x1000
KERNEL_START_ADDR equ 0x10000

USER_LOAD_SEG     equ 0x3000
USER_LOAD_ADDR    equ 0x30000

MEMORY_MAP_COUNT   equ 0x4FF0
MEMORY_MAP_BUFFER  equ 0x5000
USER_PROGRAM_SIZE  equ 0x4FE0
MAX_MEMORY_ENTRIES equ 128


stage2_start:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Keep stack safely below stage2 at 0x8000
    mov sp, 0x7C00

    ; Stage 1 passed boot drive in DL
    mov [boot_drive], dl

    sti

    ; Get E820 memory map
    call detect_memory

    ; Load kernel
    call load_kernel

    ; Load user program
    call load_user

    ; Tell kernel exact user binary size
    mov dword [USER_PROGRAM_SIZE], USER_SIZE

    call vbe_setup

    ; Enter protected mode
    jmp load_PM


; =========================================================
; E820 MEMORY MAP
; =========================================================
; =========================================================
; VBE GRAPHICS SETUP
; Find 1024x768x32 mode with linear framebuffer
; =========================================================

vbe_setup:
    ; -----------------------------------------------------
    ; Get VBE controller information
    ; Controller info buffer = physical 0x7000
    ; ES:DI = 0700:0000
    ; -----------------------------------------------------
    mov ax, 0x0700
    mov es, ax
    mov di, 0

    mov ax, 0x4F00
    int 0x10

    cmp ax, 0x004F
    jne vbe_failed

    ; Controller info +0x0E contains far pointer
    ; to the supported video mode list.
    mov si, [es:0x0E]
    mov dx, [es:0x10]

    ; FS:SI now points to mode list.
    mov fs, dx


vbe_check_mode:
    ; Get next 16-bit mode number.
    mov cx, [fs:si]

    ; 0xFFFF terminates the mode list.
    cmp cx, 0xFFFF
    je vbe_modes_done

    ; -----------------------------------------------------
    ; Get information for this mode.
    ;
    ; Mode info buffer = physical 0x7200
    ; ES:DI = 0700:0200
    ;
    ; CX already contains mode number.
    ; -----------------------------------------------------
    mov di, 0x0200

    mov ax, 0x4F01
    int 0x10

    cmp ax, 0x004F
    jne vbe_next_mode

    ; Width == 1024?
    mov ax, [es:di + 0x12]
    cmp ax, 1024
    jne vbe_next_mode

    ; Height == 768?
    mov ax, [es:di + 0x14]
    cmp ax, 768
    jne vbe_next_mode

    ; BitsPerPixel == 32?
    mov al, [es:di + 0x19]
    cmp al, 32
    jne vbe_next_mode

    ; -----------------------------------------------------
    ; ModeAttributes
    ;
    ; bit 7 = linear framebuffer available
    ; bit 0 = mode supported
    ; -----------------------------------------------------
    mov ax, [es:di]

    test ax, 0x80
    jz vbe_next_mode

    test ax, 0x01
    jz vbe_next_mode

    ; -----------------------------------------------------
    ; Save framebuffer information for kernel.
    ;
    ; Boot information:
    ; 0x4FD0 = framebuffer physical address (32 bit)
    ; 0x4FD4 = pitch                        (16 bit)
    ; 0x4FD6 = width                        (16 bit)
    ; 0x4FD8 = height                       (16 bit)
    ; 0x4FDA = bits per pixel               (8 bit)
    ; 0x4FDC = VBE mode                     (16 bit)
    ; -----------------------------------------------------

    ; GS:0 = physical 0x4FD0
    mov bx, 0x04FD
    mov gs, bx

    ; Framebuffer physical address
    mov eax, [es:di + 0x28]
    mov [gs:0], eax

    ; Pitch
    mov ax, [es:di + 0x10]
    mov [gs:4], ax

    ; Width
    mov ax, [es:di + 0x12]
    mov [gs:6], ax

    ; Height
    mov ax, [es:di + 0x14]
    mov [gs:8], ax

    ; BPP
    mov al, [es:di + 0x19]
    mov [gs:10], al

    ; CX still contains selected VBE mode
    mov [gs:12], cx

    ; -----------------------------------------------------
    ; Set selected mode and request linear framebuffer.
    ; -----------------------------------------------------
    mov bx, cx
    or bx, 0x4000

    mov ax, 0x4F02
    int 0x10

    cmp ax, 0x004F
    jne vbe_failed

    ret


vbe_next_mode:
    add si, 2
    jmp vbe_check_mode


vbe_modes_done:
    ; Reached 0xFFFF without finding desired mode.
    jmp vbe_failed


vbe_failed:
    cli

.vbe_hang:
    hlt
    jmp .vbe_hang

    
detect_memory:
    xor ax, ax
    mov es, ax

    mov di, MEMORY_MAP_BUFFER

    xor ebx, ebx
    xor bp, bp

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24

    mov dword [es:di + 20], 1

    int 0x15

    jc .finished

    cmp eax, 0x534D4150
    jne .failed

    inc bp
    add di, 24

    cmp bp, MAX_MEMORY_ENTRIES
    jae .finished

    test ebx, ebx
    jne .next_entry

.finished:
    mov [MEMORY_MAP_COUNT], bp
    ret

.failed:
    xor bp, bp
    mov [MEMORY_MAP_COUNT], bp
    ret


; =========================================================
; LOAD KERNEL
; =========================================================

load_kernel:
    xor ax, ax
    mov ds, ax

    mov si, kernel_disk_packet
    mov dl, [boot_drive]

    mov ah, 0x42
    int 0x13

    jc disk_read_error
    ret


; =========================================================
; LOAD USER PROGRAM
; =========================================================

load_user:
    xor ax, ax
    mov ds, ax

    mov si, user_disk_packet
    mov dl, [boot_drive]

    mov ah, 0x42
    int 0x13

    jc disk_read_error
    ret


; =========================================================
; DISK ERROR
; =========================================================

disk_read_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10

.hang:
    cli
    hlt
    jmp .hang


; =========================================================
; ENTER PROTECTED MODE
; =========================================================

load_PM:
    cli

    xor ax, ax
    mov ds, ax

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    jmp CODE_OFFSET:PModeMain


; =========================================================
; GDT
; =========================================================

gdt_start:

    ; Null descriptor
    dd 0x00000000
    dd 0x00000000

    ; Kernel code - selector 0x08
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Kernel data - selector 0x10
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

    ; User code - selector 0x18 | RPL3 = 0x1B
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11111010b
    db 11001111b
    db 0x00

    ; User data - selector 0x20 | RPL3 = 0x23
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11110010b
    db 11001111b
    db 0x00

gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


; =========================================================
; DATA
; =========================================================

boot_drive:
    db 0


kernel_disk_packet:
    db 0x10
    db 0x00

    dw KERNEL_SECTORS

    dw 0x0000
    dw KERNEL_LOAD_SEG

    ; LBA 0 = stage1
    ; LBA 1 = stage2
    ; LBA 2 = kernel
    dq 2


user_disk_packet:
    db 0x10
    db 0x00

    dw USER_SECTORS

    dw 0x0000
    dw USER_LOAD_SEG

    dq USER_LBA


; =========================================================
; PROTECTED MODE
; =========================================================

[BITS 32]

PModeMain:
    mov ax, DATA_OFFSET

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x9C00
    mov esp, ebp

    jmp CODE_OFFSET:KERNEL_START_ADDR

times 512 - ($ - $$) db 0