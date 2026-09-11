#ifndef FBTERMINAL_H
#define FBTERMINAL_H


#define TERMINAL_CELL_WIDTH  9
#define TERMINAL_CELL_HEIGHT 10

typedef struct{
    char character;
    unsigned int fg_color;
    unsigned int bg_color;
} TerminalCell;


extern unsigned int fg_color_sys;
extern unsigned int bg_color_sys;
TerminalCell *GetTerminalCell(unsigned int column, unsigned int row);
void Terminal_Init();
void UpdateTerminal();
void FbWriteChar(char c, unsigned int deferUpdate);
void FbWriteString(const char *text);
void FbNewLine();
void FbScroll();
void UpdateTerminalCell(unsigned int x, unsigned int y);
void FbBackspace();
void FbPrintPrompt();
void FbMakeColor(unsigned int fg_color, unsigned int bg_color);
void FbClear();
void FbWriteInt(unsigned int value);
void FbWriteHex(unsigned int value);
#endif