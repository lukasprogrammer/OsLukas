#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "pit.h"
#include "memory.h"
#include "paging.h"
#include "heap.h"
#include "task.h"
#include "gdt.h"
#include "keyboard.h"
#include "graphics/graphics.h"

#define FRAMEBUFFER_ADDR_PTR ((volatile unsigned int *)0x4FD0)
#define FRAMEBUFFER_PITCH_PTR ((volatile unsigned short *)0x4FD4)
#define FRAMEBUFFER_WIDTH_PTR ((volatile unsigned short *)0x4FD6)
#define FRAMEBUFFER_HEIGHT_PTR ((volatile unsigned short *)0x4FD8)
#define FRAMEBUFFER_BPP_PTR ((volatile unsigned char *)0x4FDA)
#define FRAMEBUFFER_VBE_MODE_PTR ((volatile unsigned short *)0x4FDC)

#define FRAMEBUFFER_VIRTUAL_ADDR 0xE0000000

unsigned int framebuffer_phys;
unsigned short framebuffer_pitch;
unsigned short framebuffer_width;
unsigned short framebuffer_height;
unsigned char framebuffer_bpp;
unsigned short framebuffer_vbe_mode;

unsigned int framebuffer_size;
unsigned int framebuffer_phys_aligned;
unsigned int framebuffer_offset;
unsigned int framebuffer_pages;

unsigned char *framebuffer;
unsigned int *framebuffer32;





void DummyUserTask(void);
void kernel_main(){
    idt_init();
    pic_remap();
    pit_init(100);
    InitializeBitmap();
    PagingInit();







    HeapInit();
    gdt_init();
    
    unsigned int new_stack = CreateKernelStack();

    if (new_stack == 0) {
        WriteTerminal("Failed to create kernel stack\n");

        while (1) {
        }
    }
    __asm__ volatile("sti");

    switch_stack(new_stack, kernel_after_stack_switch);

    while (1) {
    }
}

void kernel_after_stack_switch(void)
{


     framebuffer_phys = *FRAMEBUFFER_ADDR_PTR;
    framebuffer_pitch = *FRAMEBUFFER_PITCH_PTR;
    framebuffer_width = *FRAMEBUFFER_WIDTH_PTR;
    framebuffer_height = *FRAMEBUFFER_HEIGHT_PTR;
    framebuffer_bpp = *FRAMEBUFFER_BPP_PTR;
    framebuffer_vbe_mode = *FRAMEBUFFER_VBE_MODE_PTR;

    framebuffer_size =
        framebuffer_pitch * framebuffer_height;

    framebuffer_phys_aligned =
        framebuffer_phys - framebuffer_phys % PAGE_SIZE;

    framebuffer_offset =
        framebuffer_phys - framebuffer_phys_aligned;

    framebuffer_pages =
        (framebuffer_size +
         framebuffer_offset +
         PAGE_SIZE - 1) / PAGE_SIZE;

    framebuffer =
        (unsigned char *)
        (FRAMEBUFFER_VIRTUAL_ADDR + framebuffer_offset);

    framebuffer32 =
        (unsigned int *)framebuffer;


    for(unsigned int i = 0; i < framebuffer_pages; i++){
        unsigned int virt = FRAMEBUFFER_VIRTUAL_ADDR + i * PAGE_SIZE;
        unsigned int phys = framebuffer_phys_aligned + i * PAGE_SIZE;
        
        MapPage(virt, phys, PAGE_PRESENT|PAGE_WRITE);
    }


    DrawLine(100, 100, 500, 100, 0x00FFFFFF);  // horizontal
    DrawLine(100, 100, 100, 500, 0x00FF0000);  // vertical
    DrawLine(100, 100, 500, 500, 0x0000FF00);  // diagonal down-right
    DrawLine(500, 500, 100, 200, 0x000000FF);  // back up-left

    WriteTerminal("New kernel stack active!\n");
    Make_color(VGA_GREEN, system_bgcolor);
    WriteTerminal("Kernel started succesfully\n");
    Make_color(system_fgcolor, system_bgcolor);
    WriteTerminal("WELCOME TO LUKASOS, LIGHTWEIGHT AND VERY FAST!\n");


    Task *a = CreateTask(TaskA);
    Task *b = CreateTask(IdleTask);
    if (a == 0 || b == 0) {
        WriteTerminal("Task creation failed");
        while (1);
    }
    KeyboardClearQueue();
    KeyboardSetUserMode(1);
    CreateUserTask(DummyUserTask);

    PrintPrompt();
    StartScheduler();

}

void TaskA(void){
    
}

void IdleTask(void)
{
    while (1) {
        asm volatile("hlt");
    }
}

void DummyUserTask(void)
{
    while (1) {
    }
}




