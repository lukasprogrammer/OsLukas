#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "pit.h"

void kernel_main(){
    Make_color(VGA_GREEN, VGA_BLACK);
    WriteTerminal("Kernel started succesfully\n");
    Make_color(VGA_WHITE, VGA_BLACK);
    WriteTerminal("Welcome!\n");

    idt_init();
    pic_remap();
    pit_init(100);


    __asm__ volatile("sti");



    volatile unsigned int last_ticks = 0;

    while (1) {
//            if(ticks != last_ticks){
//                 last_ticks = ticks;
            

//           if(last_ticks % 100 == 0){
 //                WriteTerminal("Ticks: ");
 //                WriteInt(ticks);
  //               WriteTerminal("\n");
  //           }
  //       }
    }
}




