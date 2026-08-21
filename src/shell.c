#include "terminal.h"
#include "interrupts.h"
#include "shell.h"
#include "kernel.h"
#include "memory.h"
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
        Make_color(VGA_LIGHT_BLUE, system_bgcolor);
        WriteTerminal("Available Commands:\nhelp  - Show this help\nclear  - Clear the screen\nticks  - Show timer ticks\necho <msg>  - Echo a message\n");
        Make_color(system_fgcolor, system_bgcolor);

    }else if(StringEquals(argv[0], "color")){
        if(argc == 3){
            if(ParseColor(argv[1]) != -1 && ParseColor(argv[2]) != -1){
                Make_color(ParseColor(argv[1]), ParseColor(argv[2]));
                system_fgcolor = ParseColor(argv[1]);
                system_bgcolor = ParseColor(argv[2]);
            }else{
                WriteTerminal("Unknown color: ");
                WriteTerminal(argv[1]);
                WriteTerminal(" ");
                WriteTerminal(argv[2]);
                WriteTerminal("\n");
            }

        }else{
            WriteTerminal("Incorrect amount of arguments.\n");
        }
        

    }else if(StringEquals(argv[0], "memmap")){
        if(argc == 2){
            PrintMemoryMap(ReadInt(argv[1]));
        }else if(argc == 1){
            PrintMemoryMap(0);
        }else{
            WriteTerminal("Invalid Amount of arguments\n");

        }
    }else if(StringEquals(argv[0], "clear")){
        clear_terminal();

    }else if(StringEquals(argv[0], "alloc")){
        unsigned int address = AllocPage();
        if(address == 0){
            Make_color(VGA_RED, system_bgcolor);
            WriteTerminal("Memory Error");
            Make_color(system_bgcolor, system_fgcolor);
            WriteTerminal("Out of Physical Memory\n");


        }else{
            WriteTerminal("Allocated page at: ");
            WriteHex(address);
            WriteTerminal("\n");
        }

    }else if(StringEquals(argv[0], "free")){
        if(argc == 2){
            if(FreePage(ReadInt(argv[1]))){
                WriteTerminal("Page freed succesfully!\n");
            }else{
                WriteTerminal("Incorrect address format\n");
            }
        }else{
            WriteTerminal("Invalid Amount of arguments\n");
        }

    }else if(StringEquals(argv[0], "ticks")){
        WriteTerminal("Current ticks Passed: ");
        WriteInt(ticks);
        WriteTerminal("\n");

    }else if(StringEquals(argv[0], "echo")){
        for(int i = 1; i < argc; i++){
            WriteTerminal(argv[i]);
            if (i < argc - 1) {
                WriteTerminal(" ");
            }
        }

        WriteTerminal("\n");
    }else if(argv[0][0] == '\0'){
        //nothing
    }else{
        Make_color(VGA_LIGHT_RED, system_bgcolor);
        WriteTerminal("Unknown Command: ");
        Make_color(system_fgcolor, system_bgcolor);
        WriteTerminal(argv[0]);
        WriteTerminal("\n");
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
int ParseColor(const char *name){
    int i = 0;
    for(i = 0; i < 16; i++){
        if(StringEquals(vga_color_names[i], name)){
            break;
        }
        
    }
    if(i == 16){
        return -1;
    }else{
        return i;

    }

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