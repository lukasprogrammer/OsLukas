#ifndef INTERRUPTS_H
#define INTERRUPTS_H
struct registers {
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int int_no;
    unsigned int err_code;

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
};
struct user_registers {
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int int_no;
    unsigned int err_code;

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;

    unsigned int useresp;
    unsigned int ss;
};
extern volatile unsigned int ticks;

void exception_handler(struct registers *regs);

static void syscall_handler(struct registers *regs);


#endif