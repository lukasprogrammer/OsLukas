#ifndef GRAPHICS_H
#define GRAPHICS_H

extern unsigned char *framebuffer;
extern unsigned short framebuffer_pitch;
extern unsigned short framebuffer_width;
extern unsigned short framebuffer_height;
extern unsigned char framebuffer_bpp;

void PutPixel(int x, int y, unsigned int color);
void DrawRect(int x, int y, int width, int height, unsigned int color);
void DrawLine(int x1, int y1, int x2, int y2, unsigned int color);

void DrawChar(int x, int y, char c, unsigned int color);
void DrawString(int x, int y, const char *text, unsigned int color);

#endif