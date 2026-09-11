#include "graphics.h"
#include "../kernel.h"
#include "font.h"



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

void DrawChar(int x, int y, char c, unsigned int color){
    for(int row = 0; row < 8; row++){
        unsigned char rowData = font[(unsigned char)c][row];
        for(int col = 0; col < 8; col++){
            
            if((rowData)&(0x80 >> col)){
                PutPixel(x + col, y + row, color);
            }
        }
    }
}


void DrawString(int x, int y, const char *text, unsigned int color){
    int z = 0;
    int currentx = x;
    int currenty = y;

    while(text[z] != '\0'){
        if(text[z] == '\n' || currentx + 8 >= framebuffer_width){
            currentx = x;
            currenty += 10;
        }else{
            DrawChar(currentx, currenty, text[z], color);
            currentx += 9;
        }



        z++;
    }
}


