#ifndef KERNEL_H
#define KERNEL_H

void kernel_main(void);
void switch_stack(unsigned int new_stack, void (*entry)(void));
void kernel_after_stack_switch(void);
void TaskA(void);
void TaskB(void);
void IdleTask(void);








#endif