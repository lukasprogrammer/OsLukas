#ifndef KERNEL_H
#define KERNEL_H

void kernel_main(void);
void switch_stack(unsigned int new_stack, void (*entry)(void));
void kernel_after_stack_switch(void);
void RenderTask(void);
void TaskB(void);
void IdleTask(void);
extern volatile unsigned int mouse_packet_count;
extern unsigned int framebuffer_size;
extern int just_pressed;
extern int just_released;
extern int previous_left_down;








#endif