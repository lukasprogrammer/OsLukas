#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "pit.h"
#include "memory.h"
#include "paging.h"
#include "heap.h"
void kernel_main(){
    idt_init();
    pic_remap();
    pit_init(100);
    InitializeBitmap();
    PagingInit();
    HeapInit();
    unsigned int new_stack = CreateKernelStack();

    if (new_stack == 0) {
        WriteTerminal("Failed to create kernel stack\n");

        while (1) {
        }
    }
    __asm__ volatile("sti");




    switch_stack(new_stack, kernel_after_stack_switch);








    while (1) {
    }
}

void kernel_after_stack_switch(void)
{
    WriteTerminal("New kernel stack active!\n");
    Make_color(VGA_GREEN, system_bgcolor);
    WriteTerminal("Kernel started succesfully\n");
    Make_color(system_fgcolor, system_bgcolor);
    WriteTerminal("WELCOME TO LUKASOS, LIGHTWEIGHT AND VERY FAST!\n");
    PrintPrompt();


    while (1) {
    }
}




