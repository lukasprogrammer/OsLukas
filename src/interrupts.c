#include "interrupts.h"
#include "terminal.h"

const char *exception_messages[32] = {
    "Divide Error",                         // 0
    "Debug",                                // 1
    "Non-Maskable Interrupt",               // 2
    "Breakpoint",                           // 3
    "Overflow",                             // 4
    "BOUND Range Exceeded",                 // 5
    "Invalid Opcode",                       // 6
    "Device Not Available",                 // 7
    "Double Fault",                         // 8
    "Coprocessor Segment Overrun",           // 9
    "Invalid TSS",                          // 10
    "Segment Not Present",                  // 11
    "Stack-Segment Fault",                  // 12
    "General Protection Fault",             // 13
    "Page Fault",                           // 14
    "Reserved",                             // 15
    "x87 Floating-Point Exception",         // 16
    "Alignment Check",                      // 17
    "Machine Check",                        // 18
    "SIMD Floating-Point Exception",        // 19
    "Virtualization Exception",             // 20
    "Control Protection Exception",         // 21
    "Reserved",                             // 22
    "Reserved",                             // 23
    "Reserved",                             // 24
    "Reserved",                             // 25
    "Reserved",                             // 26
    "Reserved",                             // 27
    "Hypervisor Injection Exception",       // 28
    "VMM Communication Exception",          // 29
    "Security Exception",                   // 30
    "Reserved"                              // 31
};

void exception_handler(struct registers *regs){
    Make_color(VGA_RED, VGA_BLACK);
    WriteTerminal("CPU EXCEPTION\nNumber: ");
    WriteInt(regs->int_no);
    WriteTerminal("\nType: ");
    if (regs->int_no < 32) {
        WriteTerminal(exception_messages[regs->int_no]);
    } else {
        WriteTerminal("Unknown Exception");
    }
    WriteTerminal("\nError Code: ");
    WriteHex(regs->err_code);
    WriteTerminal("\nEIP: ");
    WriteHex(regs->eip);

    
    
    while(1){

    }
}