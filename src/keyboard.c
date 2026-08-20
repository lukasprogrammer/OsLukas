#include "keyboard.h"
#include "io.h"
#include "terminal.h"
#include "interrupts.h"
#include "shell.h"
static int shift_pressed = 0;


static char input_buffer[INPUT_SIZE];
static unsigned int input_pos = 0;
void keyboard_handler(void)
{
    unsigned char scancode = inb(0x60);
    char c;


        if(scancode ==0x2A || scancode == 0x36){
            shift_pressed = 1;
        }else if(scancode == 0xAA || scancode == 0xB6){
            shift_pressed = 0;
        }else{
            if (!(scancode & 0x80)) {
                if(shift_pressed){
                    c = scancode_to_ascii_shift[scancode];
                }else{
                    c = scancode_to_ascii[scancode];
                }

                if(c != 0){
                    
                    if(c == '\b' ){
                        if(input_pos > 0){
                            input_pos--;
                            input_buffer[input_pos ] = '\0';
                            terminal_putchar(c);
                        }
                    }else if(c == '\n'){

                        input_buffer[input_pos] = '\0';
                        NewLine();
                        ProcessCommand(input_buffer);

                        input_pos = 0;
                        input_buffer[0] = '\0';

                        PrintPrompt();
                    }else
                    {
                        if(input_pos < INPUT_SIZE -1){
                        input_buffer[input_pos]= c;
                        input_pos++;
                        input_buffer[input_pos] = '\0';
                        terminal_putchar(c);

                        }         
                    }
                }


                
            }

        }
}

