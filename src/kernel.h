#ifndef KERNEL_H
#define KERNEL_H

void kernel_main();
int terminal_pos = 0;

void NewLine(){
    terminal_pos = (terminal_pos/80 + 1)*80;
}

void terminal_putchar(char c){
        volatile unsigned short *video =
    (volatile unsigned short *)0xB8000;

    if(c == '\n'){
        NewLine();
    }else{
        video[terminal_pos] = (0x07 << 8) | c;
        terminal_pos++;
    }

    if(terminal_pos%79 == 0 && terminal_pos != 0){
        NewLine();
    }
}



void WriteTerminal(const char *msg){
    int i = 0;

    while (msg[i] != '\0') {
        terminal_putchar(msg[i]);
        i++;
    }
}



#endif