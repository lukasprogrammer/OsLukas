#include "terminal.h"
#include "interrupts.h"
#include "shell.h"
#include "kernel.h"
#include "memory.h"
#include "graphics/fbterminal.h"

int StringEquals(const char *a, const char *b){
    int i = 0;
    
    while(a[i] != '\0'){
        if(a[i] != b[i]){
            return 0;
        }
        i++;
    }
    if(b[i] != '\0'){
        return 0;
    }
    return 1;
}
void ProcessCommand(const char *input){
    char *argv[10];
    int argc = ParseCommand(input, argv, 10);
    if(StringEquals(argv[0], "help")){
        FbMakeColor(VGA_LIGHT_BLUE, bg_color_sys);
        FbWriteString("Available Commands:\nhelp  - Show this help\nclear  - Clear the screen\nticks  - Show timer ticks\necho <msg>  - Echo a message\ncolor <foreground> <background>  - Change interface color\nmemmap  - Show memory map\nalloc  - Allocate first available memory page on stack\nfree <addr>  - Free page of memory on specified memory address\n");
        FbMakeColor(fg_color_sys, bg_color_sys);

    }else if (StringEquals(argv[0], "color")) {
        if (argc == 3) {

            unsigned int fg = ParseColor(argv[1]);
            unsigned int bg = ParseColor(argv[2]);

            if (fg != 0xFFFFFFFF && bg != 0xFFFFFFFF) {
                fg_color_sys = fg;
                bg_color_sys = bg;    
                FbMakeColor(fg, bg);
            } else {
                FbWriteString("Unknown color: ");
                FbWriteString(argv[1]);
                FbWriteString(" ");
                FbWriteString(argv[2]);
                FbWriteString("\n");
            }

        } else {
            FbWriteString("Incorrect amount of arguments.\n");
        }
    }else if(StringEquals(argv[0], "memmap")){
        if(argc == 2){
            PrintMemoryMap(ReadInt(argv[1]));
        }else if(argc == 1){
            PrintMemoryMap(0);
        }else{
            FbWriteString("Invalid Amount of arguments\n");

        }
    }else if(StringEquals(argv[0], "clear")){
        FbClear();

    }else if(StringEquals(argv[0], "alloc")){
        unsigned int address = AllocPage();
        if(address == 0){
            FbMakeColor(VGA_RED, bg_color_sys);
            FbWriteString("Memory Error");
            FbMakeColor(bg_color_sys, fg_color_sys);
            FbWriteString("Out of Physical Memory\n");


        }else{
            FbWriteString("Allocated page at: ");
            FbWriteHex(address);
            FbWriteString("\n");
        }

    }else if(StringEquals(argv[0], "mouse")){
        FbWriteInt(mouse_packet_count);
        FbWriteString("\n");
    }
    else if(StringEquals(argv[0], "free")){
        if(argc == 2){
            if(FreePage(ReadInt(argv[1]))){
                FbWriteString("Page freed succesfully!\n");
            }else{
                FbWriteString("Incorrect adress - adress already freed or incorrect format\n");
            }
        }else{
            FbWriteString("Invalid Amount of arguments\n");
        }

    }else if(StringEquals(argv[0], "ticks")){
        FbWriteString("Current ticks Passed: ");
        FbWriteInt(ticks);
        FbWriteString("\n");

    }else if(StringEquals(argv[0], "echo")){
        for(int i = 1; i < argc; i++){
            FbWriteString(argv[i]);
            if (i < argc - 1) {
                FbWriteString(" ");
            }
        }

        FbWriteString("\n");
    }else if(argv[0][0] == '\0'){
        //nothing
    }else{
        FbMakeColor(VGA_LIGHT_RED, bg_color_sys);
        FbWriteString("Unknown Command: ");
        FbMakeColor(fg_color_sys, bg_color_sys);
        FbWriteString(argv[0]);
        FbWriteString("\n");
    }

}
int ParseCommand(char *input, char *argv[], int max_args){
    int i = 0;
    int argc = 0;

    while(input[i] != '\0' && argc < max_args){
        while(input[i] == ' '){
            i++;
        }

        if(input[i] == '\0'){
            break;
        }

        argv[argc] = &input[i];
        argc++;

        while (input[i] != ' ' && input[i] != '\0') {
            i++;
        }

        if (input[i] == ' ') {
            input[i] = '\0';
            i++;
        }
    }
    return argc;
    
}
unsigned int ParseColor(const char *color)
{
    if (StringEquals(color, "black") == 1)         return VGA_BLACK;
    if (StringEquals(color, "blue") == 1)          return VGA_BLUE;
    if (StringEquals(color, "green") == 1)         return VGA_GREEN;
    if (StringEquals(color, "cyan") == 1)          return VGA_CYAN;
    if (StringEquals(color, "red") == 1)           return VGA_RED;
    if (StringEquals(color, "magenta") == 1)       return VGA_MAGENTA;
    if (StringEquals(color, "brown") == 1)         return VGA_BROWN;
    if (StringEquals(color, "lightgrey") == 1)     return VGA_LIGHT_GREY;
    if (StringEquals(color, "darkgrey") == 1)      return VGA_DARK_GREY;
    if (StringEquals(color, "lightblue") == 1)      return VGA_LIGHT_BLUE;
    if (StringEquals(color, "lightgreen") == 1)     return VGA_LIGHT_GREEN;
    if (StringEquals(color, "lightcyan") == 1)      return VGA_LIGHT_CYAN;
    if (StringEquals(color, "lightred") == 1)       return VGA_LIGHT_RED;
    if (StringEquals(color, "lightmagenta") == 1)   return VGA_LIGHT_MAGENTA;
    if (StringEquals(color, "lightbrown") == 1)     return VGA_LIGHT_BROWN;
    if (StringEquals(color, "white") == 1)          return VGA_WHITE;

    return VGA_WHITE;
}
int ReadInt(const char *intvalue){
    int i = 0;
    int result = 0;
    while(intvalue[i] != '\0'){
        if(intvalue[i] >= '0' && intvalue[i] <= '9'){
            result = result * 10 + (intvalue[i] - '0');
            i++;
        }else{
            return -1;
        }

        
    }
    return result;
}