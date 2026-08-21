#ifndef SHELL_H
#define SHELL_H


int StringEquals(const char *a, const char *b);
void ProcessCommand(const char *command);
int ParseCommand(char *input, char *argv[], int max_args);
int ParseColor(const char *name);
static const char *vga_color_names[16] = {
    "black",          // 0
    "blue",           // 1
    "green",          // 2
    "cyan",           // 3
    "red",            // 4
    "magenta",        // 5
    "brown",          // 6
    "light_grey",     // 7
    "dark_grey",      // 8
    "light_blue",     // 9
    "light_green",    // 10
    "light_cyan",     // 11
    "light_red",      // 12
    "light_magenta",  // 13
    "light_brown",    // 14
    "white"           // 15
};
int ReadInt(const char *intvalue);
#endif