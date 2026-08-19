#include "kernel.h"
#include "terminal.h"
#include "idt.h"



void kernel_main(){
    Make_color(VGA_GREEN, VGA_BLACK);
    WriteTerminal("Kernel started succesfully\n");
    Make_color(VGA_WHITE, VGA_BLACK);
    WriteTerminal("Welcome!\n");

    idt_init();
    __asm__ volatile(
    "mov $0x23, %ax\n"
    "mov %ax, %ds\n"
    );




    while (1) {
    }
}




