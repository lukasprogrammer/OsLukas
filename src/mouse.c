#include "mouse.h"
#include "io.h"
#include "graphics/graphics.h"
int mouse_x = 0;
int mouse_y = 0;
unsigned char mouse_buttons = 0;

static unsigned char mouse_cycle = 0;
static unsigned char mouse_packet[3];

volatile unsigned int mouse_packet_count = 0;

static int old_mouse_x;
static int old_mouse_y;
static unsigned int saved_pixels[25];
static int cursor_drawn = 0;
volatile int mouse_moved = 0;

void MouseInit(){
    MouseWaitWrite();
    outb(0x64, 0xA8);
    MouseWaitWrite();
    outb(0x64, 0x20);
    MouseWaitRead();
    unsigned char config = inb(0x60);
    config = config | 0x02;
    MouseWaitWrite();
    outb(0x64, 0x60);
    MouseWaitWrite();
    outb(0x60, config);

    MouseWaitWrite();
    outb(0x64, 0xD4);
    MouseWaitWrite();
    outb(0x60, 0xF4);

    MouseWaitRead();
    unsigned char ack = inb(0x60);

}
void MouseHandler(){

    unsigned char data = inb(0x60);


    if(mouse_cycle == 0){
        if(!(data & 0x08)){
            return;
        }
    }

    mouse_packet[mouse_cycle] = data;

    mouse_cycle++;

    if (mouse_cycle == 3)
    {
        mouse_cycle = 0;
        
        mouse_packet_count++;





        int dx = (signed char)mouse_packet[1];
        int dy = (signed char)mouse_packet[2];

        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0){
            mouse_x = 0;
        }
        if (mouse_x >= framebuffer_width-5){
            mouse_x = framebuffer_width - 5;
        }
        if (mouse_y < 0){
            mouse_y = 0;
        }
        if (mouse_y >= framebuffer_height-5){
            mouse_y = framebuffer_height - 5;
        }


        mouse_buttons = mouse_packet[0]&0x07;
        mouse_moved = 1;



    }
}
static void MouseWaitWrite(){

    unsigned char status  = inb(0x64);
    while(status & 0x02){
        status  = inb(0x64);
    }

}
static void MouseWaitRead(){

    unsigned char status  = inb(0x64);
    while(!(status & 0x01)){
        status  = inb(0x64);
    }

}
void MouseUpdateCursor(void)
{
    if(!mouse_moved)
        return;

    __asm__ volatile("cli");

    int new_x = mouse_x;
    int new_y = mouse_y;

    mouse_moved = 0;

    /*
     * Work out one rectangle containing both
     * the old cursor and the new cursor.
     */
    int left   = new_x;
    int top    = new_y;
    int right  = new_x + 5;
    int bottom = new_y + 5;

    if(cursor_drawn)
    {
        if(old_mouse_x < left)
            left = old_mouse_x;

        if(old_mouse_y < top)
            top = old_mouse_y;

        if(old_mouse_x + 5 > right)
            right = old_mouse_x + 5;

        if(old_mouse_y + 5 > bottom)
            bottom = old_mouse_y + 5;
    }

    /*
     * Rebuild this entire region directly from
     * the clean backbuffer.
     *
     * If a pixel belongs to the NEW cursor,
     * make it white.
     *
     * Otherwise copy the clean background.
     */
    for(int y = top; y < bottom; y++)
    {
        for(int x = left; x < right; x++)
        {
            unsigned int color;

            if(x >= new_x &&
               x < new_x + 5 &&
               y >= new_y &&
               y < new_y + 5)
            {
                color = 0x00FFFFFF;
            }
            else
            {
                unsigned int offset =
                    y * framebuffer_pitch + x * 4;

                color =
                    *(unsigned int *)(backbuffer + offset);
            }

            unsigned int offset =
                y * framebuffer_pitch + x * 4;

            *(unsigned int *)(framebuffer + offset) = color;
        }
    }

    old_mouse_x = new_x;
    old_mouse_y = new_y;
    cursor_drawn = 1;

    __asm__ volatile("sti");
}