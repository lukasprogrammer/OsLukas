#ifndef MOUSE_H
#define MOUSE_H

extern int mouse_x;
extern int mouse_y;
extern unsigned char mouse_buttons;

void MouseInit(void);
void MouseHandler(void);
static void MouseWaitWrite();
static void MouseWaitRead();
void MouseUpdateCursor();
extern volatile int mouse_moved;
extern int dx;
extern int dy;
extern int mouse_delta_x;
extern int mouse_delta_y;

#endif