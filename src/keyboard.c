#include "keyboard.h"
#include "io.h"
#include "terminal.h"
static int shift_pressed = 0;

void keyboard_handler(void)
{
    unsigned char scancode = inb(0x60);
    char c;

    if (!(scancode & 0x80)) {



        if(shift_pressed){
            c = scancode_to_ascii_shift[scancode];
        }else{
            c = scancode_to_ascii[scancode];
        }


        if(c != 0){
            terminal_putchar(c);
        }


        
    }
    if(scancode ==0x2A || scancode == 0xB6){
        shift_pressed = 1;
    }else if(scancode == 0xAA || scancode == 0x86){
        shift_pressed = 0;
    }
}