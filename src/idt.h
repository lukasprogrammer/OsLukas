#ifndef IDT_H
#define IDT_H

void idt_init(void);
void idt_set_gate(int number, unsigned int handler);

struct idt_entry{
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;

}__attribute__((packed));

struct idt_ptr{
    unsigned short limit;
    unsigned int base;

}__attribute__((packed));

#endif