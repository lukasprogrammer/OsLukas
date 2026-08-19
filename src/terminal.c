#include "terminal.h"
#include "kernel.h"


int terminal_pos = 0;
unsigned char text_color = VGA_WHITE | (VGA_BLACK << 4);

void Make_color(unsigned char fg, unsigned char bg){
    text_color = fg | (bg << 4);
}

void clear_terminal(){
    for(int i = 0; i < 80*25;i++){
        VIDEO[i] = (0x07 << 8) | ' ';
    }
    terminal_pos = 0;
    update_cursor();
}

void NewLine(){
    terminal_pos = (terminal_pos/80 + 1)*80;
}

void Scroll(){
    for(int i = 0; i <= 80 * 24 -1; i++){
        VIDEO[i] = VIDEO[i + 80];
    }
    for(int i = 80 * 24; i <= 80*25-1;i++){
        VIDEO[i] = (0x07 << 8) | ' ';
    }
    terminal_pos = 80*24;
}
void terminal_putchar(char c){


    if(c == '\n'){
        NewLine();
    }else{
        VIDEO[terminal_pos] = (text_color << 8)| c;
        terminal_pos++;
    }
    if(terminal_pos >= 80*25){
        Scroll();
    }
    update_cursor();

}
void WriteTerminal(const char *msg){
    int i = 0;

    while (msg[i] != '\0') {
        terminal_putchar(msg[i]);
        i++;
    }
}

static inline void outb(unsigned short port, unsigned char value){
    __asm__ volatile("outb %0, %1"
                    :
                    : "a"(value), "Nd"(port));
}

static void update_cursor(){
    outb(0x3D4, 0x0F);
    outb(0x3D5, terminal_pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (terminal_pos >> 8) & 0xFF);
}