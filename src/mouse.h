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

#endif