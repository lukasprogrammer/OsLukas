#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void irq0(void);
extern void irq1(void);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);


void idt_init(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (unsigned int)&idt;

    
    idt_set_gate(0, (unsigned int)isr0);
    idt_set_gate(1, (unsigned int)isr1);
    idt_set_gate(2, (unsigned int)isr2);
    idt_set_gate(3, (unsigned int)isr3);
    idt_set_gate(4, (unsigned int)isr4);
    idt_set_gate(5, (unsigned int)isr5);
    idt_set_gate(6, (unsigned int)isr6);
    idt_set_gate(7, (unsigned int)isr7);
    idt_set_gate(8, (unsigned int)isr8);
    idt_set_gate(9, (unsigned int)isr9);
    idt_set_gate(10, (unsigned int)isr10);
    idt_set_gate(11, (unsigned int)isr11);
    idt_set_gate(12, (unsigned int)isr12);
    idt_set_gate(13, (unsigned int)isr13);
    idt_set_gate(14, (unsigned int)isr14);
    idt_set_gate(15, (unsigned int)isr15);
    idt_set_gate(16, (unsigned int)isr16);
    idt_set_gate(17, (unsigned int)isr17);
    idt_set_gate(18, (unsigned int)isr18);
    idt_set_gate(19, (unsigned int)isr19);
    idt_set_gate(20, (unsigned int)isr20);
    idt_set_gate(21, (unsigned int)isr21);
    idt_set_gate(22, (unsigned int)isr22);
    idt_set_gate(23, (unsigned int)isr23);
    idt_set_gate(24, (unsigned int)isr24);
    idt_set_gate(25, (unsigned int)isr25);
    idt_set_gate(26, (unsigned int)isr26);
    idt_set_gate(27, (unsigned int)isr27);
    idt_set_gate(28, (unsigned int)isr28);
    idt_set_gate(29, (unsigned int)isr29);
    idt_set_gate(30, (unsigned int)isr30);
    idt_set_gate(31, (unsigned int)isr31);

    idt_set_gate(32, (unsigned int)irq0);
    idt_set_gate(33, (unsigned int)irq1);


    __asm__ volatile("lidt %0" : : "m"(idtp));
}

void idt_set_gate(int number, unsigned int handler){
    idt[number].offset_low = handler & 0xFFFF;
    idt[number].offset_high = (handler >> 16) & 0xFFFF;
    idt[number].selector = 0x08;
    idt[number].zero = 0;
    idt[number].type_attr = 0x8E;
}

