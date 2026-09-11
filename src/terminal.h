#ifndef TERMINAL_H
#define TERMINAL_H

#define VGA_BLACK          0x00000000
#define VGA_BLUE           0x000000AA
#define VGA_GREEN          0x0000AA00
#define VGA_CYAN           0x0000AAAA
#define VGA_RED            0x00AA0000
#define VGA_MAGENTA        0x00AA00AA
#define VGA_BROWN          0x00AA5500
#define VGA_LIGHT_GREY     0x00AAAAAA
#define VGA_DARK_GREY      0x00555555
#define VGA_LIGHT_BLUE     0x005555FF
#define VGA_LIGHT_GREEN    0x0055FF55
#define VGA_LIGHT_CYAN     0x0055FFFF
#define VGA_LIGHT_RED      0x00FF5555
#define VGA_LIGHT_MAGENTA  0x00FF55FF
#define VGA_LIGHT_BROWN    0x00FFFF55
#define VGA_WHITE          0x00FFFFFF
#define VIDEO ((volatile unsigned short *)0xB8000)

extern unsigned char system_fgcolor;
extern unsigned char system_bgcolor;
void NewLine(void);
void Scroll();
void terminal_putchar(char c);
void WriteTerminal(const char* msg);
void delete_char();
void PrintPrompt();
void update_cursor();
void Make_color(unsigned char fg, unsigned char bg);
void clear_terminal();
void WriteHex(unsigned int value);
void WriteInt(unsigned int value);

#endif