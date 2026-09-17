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
#include "graphics/fbterminal.h"
#include "mouse.h"
#include "graphics/window.h"
#include "drivers/ata.h"


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
volatile int screen_dirty = 0;

int previous_left_down = 0;
int just_pressed = 0;
int just_released = 0;





void DummyUserTask(void);
void kernel_main(){
    idt_init();
    pic_remap();
    pit_init(100);
    InitializeBitmap();
    PagingInit();
    KeyboardInit();
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
void btnwindowadd(void){
    AddWindow(200, 200, 400, 150, "This is a button window");
    DrawAllWindows();
    PresentFrame();
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

    backbuffer = kmalloc(framebuffer_size);
    __asm__ volatile("cli");
    MouseInit();
    __asm__ volatile("sti");
    Terminal_Init();
    KeyboardClearQueue();
    KeyboardSetUserMode(0);
    CreateTask(RenderTask);

    
    ATA_SelectSlave();
    unsigned char ata_status = ATA_ReadStatus();

    FbWriteString("ATA slave status: ");
    FbWriteHex(ata_status);
    FbWriteString("\n");



    unsigned char disk_buffer[512];
    int result = ATA_ReadSector(0,disk_buffer);

    if(result == 0){
        FbWriteString("Disk Says: ");

        for(int i = 0; i < 19; i++){
            FbWriteChar(disk_buffer[i], 1);
        }
        FbWriteString("\n");
    }else{
        FbWriteString("ATA read Failed!\n");
    }


    FbPrintPrompt();




    UpdateTerminal();

    //TEST CODE


    int win1 = AddWindow(100, 100, 300, 300, "Test1");
    int win2 = AddWindow(500, 500, 200, 200, "Test2");
    

    ButtonAdd(20, 40, 50, 50, 0x00459204, "WIN", btnwindowadd, win1);
    

    AddLabel(80, 80, "Hayy\nMy Name Is Lukas", 0x0000FF00, win1);
    AddLabel(20, 20, "HELOOOOOOOOOOOOOOOOOOOOO", 0x00FF00BB, win2);
    AddLabel(200, 30, "Amy is\nGay", 0x00FF00FF, win1);
    DrawAllWindows();

    PresentFrame();

    //TEST CODE




    StartScheduler();

    

}

void RenderTask(void)
{
    while(1)
    {
        if(mouse_moved)
        {
            int left_down =
                mouse_buttons & 0x01;

            just_pressed = 0;
            just_released = 0;

            if(!previous_left_down && left_down)
            {
                just_pressed = 1;
            }
            else if(previous_left_down && !left_down)
            {
                just_released = 1;
            }


            int focus_changed = 0;

            if(just_pressed)
            {
                focus_changed =
                    FocusWindow();
            }


            int buttons_changed =
                UpdateTotalButtons();

            int windows_changed =
                UpdateAllWindows();


            previous_left_down =
                left_down;

            if(window_full_redraw_needed ||
               focus_changed)
            {
                DrawTerminal();
                DrawAllWindows();
                DrawGlobalButtons();
                DrawGlobalLabels();

                PresentFrame();

                window_full_redraw_needed = 0;
            }


            else if(windows_changed)
            {
                for(int i = 0; i < window_list_pos; i++)
                {
                    Window *win = &windows_total[i];

                    if(win->dirty_width > 0 &&
                    win->dirty_height > 0)
                    {
                        RedrawRect(
                            win->dirty_x,
                            win->dirty_y,
                            win->dirty_width,
                            win->dirty_height
                        );

                        win->dirty_width = 0;
                        win->dirty_height = 0;
                    }
                }
            }

            /*
             * Only button state changed.
             */
            else if(buttons_changed)
            {
                DrawGlobalButtons();

                PresentButtons();
            }


            /*
             * Cursor is always the final layer.
             */
            MouseUpdateCursor();


            mouse_moved = 0;
            mouse_delta_x = 0;
            mouse_delta_y = 0;
        }
    }
}
void IdleTask(void)
{
    while (1) {
        asm volatile("hlt");
    }
}





