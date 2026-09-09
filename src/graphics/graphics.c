#include "graphics.h"
#include "../kernel.h"
void PutPixel(int x, int y, unsigned int color){
    if(x >= framebuffer_width || y >= framebuffer_height || x < 0 || y < 0){
        return;
    }
    unsigned int offset = y * framebuffer_pitch + x * 4;

    unsigned int *pixel = (unsigned int *)(framebuffer + offset);
    *pixel = color;
}

void DrawRect(int x, int y, int width, int height, unsigned int color){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            PutPixel(x + j, y + i, color);
        }
    }
}

void DrawLine(int x1, int y1, int x2, int y2, unsigned int color){
    int dx = x2 - x1;
    int dy = y2 - y1;

    int sx;
    int sy;

    if(dx > 0){
        sx = 1;
    }else if(dx < 0){
        sx = -1;
    }else{
        sx = 0;
    }

    if(dy > 0){
        sy = 1;
    }else if(dy < 0){
        sy = -1;
    }else{
        sy = 0;
    }

    dx *= sx;
    dy *= sy;

    int x = x1;
    int y = y1;
    int err = dx - dy;

    while(1){
        PutPixel(x, y, color);

        if(x == x2 && y == y2){
            break;
        }

        int e2 = 2 * err;
        if(e2 > -dy){
            err -= dy;
            x += sx;
        }

        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }




}