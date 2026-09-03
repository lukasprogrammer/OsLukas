[BITS 32]

global gdt_load

gdt_load:
    mov eax, [esp + 4]

    lgdt [eax]
        ; Reload kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS using a far jump
    jmp 0x08:.reload_cs

.reload_cs:
    ret

global tss_load

tss_load:
    mov ax, 0x28
    ltr ax
    ret