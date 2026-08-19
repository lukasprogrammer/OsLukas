#include "kernel.h"
#include "terminal.h"



void kernel_main(){
    Make_color(VGA_GREEN, VGA_BLACK);
    WriteTerminal("Kernel started succesfully\n");
    Make_color(VGA_WHITE, VGA_BLACK);
    WriteTerminal("Welcome!\n");
    

    while (1) {
    }
}




