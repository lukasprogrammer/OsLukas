#include "interrupts.h"
#include "terminal.h"
#include "io.h"
#include "keyboard.h"
#include "task.h"
#include "paging.h"
#include "memory.h"
#include "mouse.h"
#include "graphics/fbterminal.h"


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
    FbWriteString("CPU EXCEPTION\nNumber: ");
    FbWriteInt(regs->int_no);
    FbWriteString("\nType: ");
    if (regs->int_no == 14) {
        unsigned int fault_address;

        asm volatile(
            "mov %%cr2, %0"
            : "=r"(fault_address)
        );

        FbWriteString("Fault Address: ");
        WriteHex(fault_address);
        FbWriteString("\n");
        
    } else if(regs->int_no < 32){
        FbWriteString(exception_messages[regs->int_no]);
    }else {
        FbWriteString("Unknown Exception");
    }
    FbWriteString("\nError Code: ");
    FbWriteHex(regs->err_code);
    FbWriteString("\nEIP: ");
    WriteHex(regs->eip);

    
    
    while(1){

    }
}
volatile unsigned int ticks = 0;
unsigned int irq_handler(struct registers *regs){
    if(regs->int_no == 32){
        ticks++;


        unsigned int new_esp = Schedule((unsigned int)regs);

        outb(0x20, 0x20);

        return new_esp;
    }
    if(regs->int_no == 33){
        keyboard_handler();
    }
    if(regs->int_no == 0x2c){
        MouseHandler();

        outb(0xA0, 0x20);  // EOI to slave
        outb(0x20, 0x20);  // EOI to master
    }
    if (regs->int_no == 48) {
        return Schedule((unsigned int)regs);
    }
    if (regs->int_no == 128) {
        if (regs->eax == 0) {
            unsigned int addr = regs->ebx;
            unsigned int len = regs->ecx;

            if (len == 0) {
                regs->eax = 0;
                return (unsigned int)regs;
            }

            if (addr > 0xFFFFFFFFu - (len - 1)) {
                regs->eax = 0xFFFFFFFF;
                return (unsigned int)regs;
            }

            unsigned int page =
                addr & 0xFFFFF000;

            unsigned int end =
                addr + len - 1;

            unsigned int last_page =
                end & 0xFFFFF000;

            while(1){

                if(!IsUserAddress(page)){  
                    regs->eax = 0xFFFFFFFF;
                    return (unsigned int)regs;
                }

                if(page == last_page){
                    break;
                }
                page += PAGE_SIZE;
            }

            char *str = (char *)addr;

            for (unsigned int i = 0; i < len; i++) {
                terminal_putchar(str[i]);
            }
            

            regs->eax = len;    

            return (unsigned int)regs;
        }
        if(regs->eax == 1){
            MarkCurrentTaskDead();
            KeyboardSetUserMode(0);
            PrintPrompt();
            return Schedule((unsigned int)regs);
        }
        if(regs->eax == 2){
            char c = KeyboardReadChar();
            if(c != 0){
                regs->eax = (unsigned int)c;
                return (unsigned int)regs;
            }
            KeyboardWaitForInput();
            MarkCurrentTaskBlocked();
            regs->eax = 0;
            return Schedule((unsigned int)regs);

            
        }if (regs->eax == 3) {
            KeyboardClearQueue();
            regs->eax = 0;
            return (unsigned int)regs;
        }
        if (regs->eax == 4) {
            clear_terminal();
            return (unsigned int)regs;
        }
        if (regs->eax == 5) {
            delete_char();
            update_cursor();
            return (unsigned int)regs;
        }

    }



    outb(0x20, 0x20);
    return (unsigned int)regs;
}

static void syscall_handler(struct registers *regs){

}