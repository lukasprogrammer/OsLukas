[BITS 16]
[ORG 0x7C00]



CODE_OFFSET equ 0x08
DATA_OFFSET equ 0x10

; Kernel will be loaded to:
; 0x1000:0x0000 = physical address 0x10000
KERNEL_LOAD_SEG   equ 0x1000
KERNEL_START_ADDR equ 0x10000

; E820 memory map storage
MEMORY_MAP_COUNT   equ 0x4FF0
MEMORY_MAP_BUFFER  equ 0x5000
MAX_MEMORY_ENTRIES equ 128


; =========================================================
; BOOTLOADER ENTRY
; =========================================================

start:
    cli

    ; Set up real-mode segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Stack grows downward from 0x7C00
    mov sp, 0x7C00

    ; BIOS gives us the boot drive in DL.
    ; Save it before BIOS calls can change DL.
    mov [boot_drive], dl

    sti


    ; -----------------------------------------------------
    ; Set VGA 80x25 text mode
    ; -----------------------------------------------------

    mov ax, 0x0003
    int 0x10


    ; -----------------------------------------------------
    ; Get BIOS physical memory map
    ; -----------------------------------------------------

    call detect_memory


    ; -----------------------------------------------------
    ; Check whether BIOS supports INT 13h extensions / LBA
    ; -----------------------------------------------------

    call check_lba


    ; -----------------------------------------------------
    ; Load kernel from disk
    ; -----------------------------------------------------

    call load_kernel


    ; Kernel loaded successfully.
    ; Switch into protected mode.
    jmp load_PM



; =========================================================
; E820 MEMORY MAP
; =========================================================

detect_memory:

    ; E820 writes entries to ES:DI.
    xor ax, ax
    mov es, ax

    mov di, MEMORY_MAP_BUFFER

    ; EBX must be zero for the first E820 call.
    xor ebx, ebx

    ; BP will count how many entries BIOS returns.
    xor bp, bp


.next_entry:

    ; EAX = E820h
    mov eax, 0xE820

    ; Required signature: "SMAP"
    mov edx, 0x534D4150

    ; Ask BIOS for a 24-byte entry
    mov ecx, 24

    ; Initialize extended attributes field
    mov dword [es:di + 20], 1

    int 0x15

    ; Carry means error/end
    jc .finished

    ; BIOS must return EAX = "SMAP"
    cmp eax, 0x534D4150
    jne .failed

    ; Count this entry
    inc bp

    ; Move to next 24-byte entry
    add di, 24

    ; Prevent our buffer from growing forever
    cmp bp, MAX_MEMORY_ENTRIES
    jae .finished

    ; BIOS returns EBX = 0 when no more entries remain
    test ebx, ebx
    jne .next_entry


.finished:

    ; Store number of returned entries at 0x4FF0
    mov [MEMORY_MAP_COUNT], bp

    ret


.failed:

    ; A count of zero tells the kernel E820 failed
    xor bp, bp
    mov [MEMORY_MAP_COUNT], bp

    ret



; =========================================================
; CHECK INT 13h EXTENSIONS
; =========================================================

check_lba:

    ; Restore DS because we're about to access boot_drive
    xor ax, ax
    mov ds, ax

    mov dl, [boot_drive]

    ; BIOS extension installation check
    mov ah, 0x41
    mov bx, 0x55AA

    int 0x13

    ; Carry means extensions unavailable
    jc disk_read_error

    ; BIOS should swap signature to AA55
    cmp bx, 0xAA55
    jne disk_read_error

    ; CX bit 0 = extended disk access supported
    test cx, 1
    jz disk_read_error

    ret



; =========================================================
; LOAD KERNEL USING LBA
; =========================================================

load_kernel:

    ; INT 13h AH=42 expects the Disk Address Packet at DS:SI
    xor ax, ax
    mov ds, ax

    mov si, disk_packet

    ; Use the actual drive we booted from
    mov dl, [boot_drive]

    ; Extended read
    mov ah, 0x42
    int 0x13

    ; BIOS sets carry on failure
    jc disk_read_error

    ret



; =========================================================
; DISK ERROR
; =========================================================

disk_read_error:

    ; We are still in real mode here, so BIOS video output
    ; is still available.
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

    ; Make sure DS is correct before loading the GDT
    xor ax, ax
    mov ds, ax

    lgdt [gdt_descriptor]

    ; Set PE bit in CR0
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Far jump reloads CS and enters our 32-bit code segment
    jmp CODE_OFFSET:PModeMain



; =========================================================
; GLOBAL DESCRIPTOR TABLE
; =========================================================

gdt_start:

    ; Null descriptor
    dd 0x00000000
    dd 0x00000000


    ; -----------------------------------------------------
    ; Code segment
    ; selector = 0x08
    ; -----------------------------------------------------

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00


    ; -----------------------------------------------------
    ; Data segment
    ; selector = 0x10
    ; -----------------------------------------------------

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00


gdt_end:


gdt_descriptor:

    dw gdt_end - gdt_start - 1
    dd gdt_start



; =========================================================
; BOOTLOADER DATA
; =========================================================

boot_drive:
    db 0


; ---------------------------------------------------------
; Disk Address Packet for INT 13h AH=42
;
; LBA 0 = boot sector
; LBA 1 = first sector of kernel.bin
;
; destination = 0x1000:0x0000 = physical 0x10000
; ---------------------------------------------------------

disk_packet:

    db 0x10                ; DAP size = 16 bytes
    db 0x00                ; reserved

    dw KERNEL_SECTORS      ; number of sectors to read

    dw 0x0000              ; destination offset
    dw KERNEL_LOAD_SEG     ; destination segment

    dq 0x0000000000000001  ; starting LBA = 1



; =========================================================
; 32-BIT PROTECTED MODE
; =========================================================

[BITS 32]

PModeMain:

    ; Load our data selector into all data segment registers
    mov ax, DATA_OFFSET

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax


    ; Set up protected-mode stack
    mov ebp, 0x9C00
    mov esp, ebp


    ; Kernel is below 1 MiB at 0x10000,
    ; so A20 isn't required yet.


    ; Jump to kernel.asm at physical 0x10000
    jmp CODE_OFFSET:KERNEL_START_ADDR



; =========================================================
; BOOT SIGNATURE
; =========================================================

times 510 - ($ - $$) db 0

dw 0xAA55