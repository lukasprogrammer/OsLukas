#include "fbterminal.h"
#include "graphics.h"
#include "heap.h"
#include "terminal.h"
unsigned int terminal_cols;
unsigned int terminal_rows;

TerminalCell *terminalcells;
unsigned int cursor_pos;

unsigned int fg_color_sys;
unsigned int bg_color_sys;

unsigned int fgcolor;
unsigned int bgcolor;

const char Fbhex[]= "0123456789ABCDEF";


void Terminal_Init(){
    terminal_cols = framebuffer_width/TERMINAL_CELL_WIDTH;
    terminal_rows = framebuffer_height/TERMINAL_CELL_HEIGHT; 
    unsigned int cursor_pos = 0;
    fg_color_sys = 0x00FFFFFF;
    bg_color_sys = 0x00000000;

    fgcolor = 0x00FFFFFF;
    bgcolor = 0x00000000;
    

    terminalcells = kmalloc(sizeof(TerminalCell)*terminal_cols*terminal_rows);

    if (terminalcells == 0) {
        return;
    }
    TerminalCell *cell;

    for(unsigned int col = 0; col < terminal_cols; col++){
        for(unsigned int row = 0; row < terminal_rows; row++){
            cell = GetTerminalCell(col, row);
            cell->character = ' ';
            cell->fg_color = 0x00FFFFFF;
            cell->bg_color = 0x00000000;
        }
    }  
}
void FbCheckScroll(){
    if(cursor_pos >= terminal_cols * terminal_rows){
        FbScroll();
    }   
}

TerminalCell *GetTerminalCell(unsigned int column, unsigned int row){
    if(column >= terminal_cols || row >= terminal_rows){
        return 0;
    }
    unsigned int index = row * terminal_cols + column;
    return &terminalcells[index];
}
void UpdateTerminal(){
    TerminalCell *cell;

    for(unsigned int x = 0; x < terminal_cols; x++){
        for(unsigned int y = 0; y < terminal_rows; y++){
            cell = GetTerminalCell(x, y);
            DrawRect(x*TERMINAL_CELL_WIDTH, y* TERMINAL_CELL_HEIGHT,  TERMINAL_CELL_WIDTH,  TERMINAL_CELL_HEIGHT, cell->bg_color);
            DrawChar(x* TERMINAL_CELL_WIDTH, y* TERMINAL_CELL_HEIGHT, cell->character,cell->fg_color);
        }
    }
}
void FbWriteChar(char c, unsigned int deferUpdate)
{
    int x = cursor_pos % terminal_cols;
    int y = cursor_pos / terminal_cols;

    TerminalCell *cell = GetTerminalCell(x, y);

    if (cell == 0) {
        return;
    }

    cell->fg_color = fgcolor;
    cell->bg_color = bgcolor;
    cell->character = c;

    cursor_pos++;
    FbCheckScroll();

    if(deferUpdate){
        return;
    }
    UpdateTerminalCell(x, y);
}

void FbWriteString(const char *text){
    int i = 0;
    while(text[i] != '\0'){
        if(text[i] == '\n'){
            FbNewLine();
        }else{
            FbWriteChar(text[i], 1);
            
        }
        i++;

    }
    UpdateTerminal();
}

void FbScroll(){
    for(int y = 1; y < terminal_rows; y++){
        for(int x = 0; x < terminal_cols; x++){
            TerminalCell *dest = GetTerminalCell(x, y - 1);
            TerminalCell *src  = GetTerminalCell(x, y);

            *dest = *src;
        }
    }
    for(int x = 0; x < terminal_cols; x++){
        TerminalCell *cell = GetTerminalCell(x, terminal_rows -1 );
        cell->character = ' ';
        cell->bg_color = bg_color_sys;
        cell->fg_color = fg_color_sys;
    }
    cursor_pos -= terminal_cols;
}
void FbNewLine(){
    cursor_pos += (terminal_cols - (cursor_pos % terminal_cols));
    FbCheckScroll();
}

void UpdateTerminalCell(unsigned int x, unsigned int y)
{
    TerminalCell *cell = GetTerminalCell(x, y);

    if (cell == 0) {
        return;
    }

    DrawRect(
        x * TERMINAL_CELL_WIDTH,
        y * TERMINAL_CELL_HEIGHT,
        TERMINAL_CELL_WIDTH,
        TERMINAL_CELL_HEIGHT,
        cell->bg_color
    );

    DrawChar(
        x * TERMINAL_CELL_WIDTH,
        y * TERMINAL_CELL_HEIGHT,
        cell->character,
        cell->fg_color
    );
}
void FbBackspace(){
    unsigned int cursorx = (cursor_pos - 1) % terminal_cols;
    unsigned int cursory = (cursor_pos - 1)/terminal_cols;

    TerminalCell *cell = GetTerminalCell(cursorx, cursory);

    cell->character = ' ';
    cell->bg_color = VGA_BLACK;
    cell->fg_color = fg_color_sys;

    cursor_pos -= 1;
    UpdateTerminalCell(cursorx, cursory);
}

void FbPrintPrompt(){
    FbWriteString("LukasOS>");
}

void FbMakeColor(unsigned int fg_color, unsigned int bg_color){
    bgcolor = bg_color;
    fgcolor = fg_color;
}
void FbClear(){
    TerminalCell *cell;

    for(unsigned int col = 0; col < terminal_cols; col++){
        for(unsigned int row = 0; row < terminal_rows; row++){
            cell = GetTerminalCell(col, row);
            cell->character = ' ';
            cell->fg_color = 0x00FFFFFF;
            cell->bg_color = 0x00000000;
        }
    }  
    cursor_pos = 0;
    UpdateTerminal();
}

void FbWriteHex(unsigned int value){
    FbWriteString("0x");
    for(int i = 28; i >= 0; i = i - 4){
        unsigned int digit = (value >> i & 0xF);

        FbWriteChar(Fbhex[digit], 1);
    }
    UpdateTerminal();
}

void FbWriteInt(unsigned int value){
    char digits[12];
    int i = 0;
    if(value != 0){
        while(value > 0){
            digits[i] = (value % 10) + '0';
            value = value/10;
            i++;
        }
    }else{
        digits[0] = '0';
        i++;
    }
    i--;
    
    while(i >= 0){
        FbWriteChar(digits[i], 1);
        i--;
    }
    UpdateTerminal();

}

