#ifndef TERMINAL_H
#define TERMINAL_H

#define VGA_BLACK         0
#define VGA_BLUE          1
#define VGA_GREEN         2
#define VGA_CYAN          3
#define VGA_RED           4
#define VGA_MAGENTA       5
#define VGA_BROWN         6
#define VGA_LIGHT_GREY    7
#define VGA_DARK_GREY     8
#define VGA_LIGHT_BLUE    9
#define VGA_LIGHT_GREEN   10
#define VGA_LIGHT_CYAN    11
#define VGA_LIGHT_RED     12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN   14
#define VGA_WHITE         15
#define VIDEO ((volatile unsigned short *)0xB8000)

extern unsigned char system_fgcolor;
extern unsigned char system_bgcolor;
void NewLine(void);
void Scroll();
void terminal_putchar(char c);
void WriteTerminal(const char* msg);
void delete_char();
void PrintPrompt();
static void update_cursor();
void Make_color(unsigned char fg, unsigned char bg);
void clear_terminal();
void WriteHex(unsigned int value);
void WriteInt(unsigned int value);

#endif