ENTRY(_start)
OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x10000;

    kernel_start = .;

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }

    kernel_end = .;
}