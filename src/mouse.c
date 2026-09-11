#include "mouse.h"
#include "io.h"
#include "graphics/graphics.h"
int mouse_x = 0;
int mouse_y = 0;
unsigned char mouse_buttons = 0;

static unsigned char mouse_cycle = 0;
static unsigned char mouse_packet[3];

volatile unsigned int mouse_packet_count = 0;

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
        if (mouse_x >= framebuffer_width){
            mouse_x = framebuffer_width - 1;
        }
        if (mouse_y < 0){
            mouse_y = 0;
        }
        if (mouse_y >= framebuffer_height){
            mouse_y = framebuffer_height - 1;
        }

        unsigned char mouse_buttons = mouse_packet[0]&0x07;



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