#include "io.h"

void pit_init(unsigned int freq){
    outb(0x43, 0x36);
    unsigned int divisor = 1193182 / freq;
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF); 
}