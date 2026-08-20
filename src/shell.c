#include "terminal.h"
#include "interrupts.h"
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
void ProcessCommand(const char *command){
    if(StringEquals(command, "help")){
        Make_color(VGA_LIGHT_BLUE, VGA_BLACK);
        WriteTerminal("Available Commands:\nhelp  - Show this help\nclear  - Clear the screen\nticks  - Show timer ticks\n");
        Make_color(VGA_WHITE, VGA_BLACK);

    }else if(StringEquals(command, "clear")){
        clear_terminal();

    }else if(StringEquals(command, "ticks")){
        WriteTerminal("Current ticks Passed: ");
        WriteInt(ticks);
        WriteTerminal("\n");

    }else if(command[0] == '\0'){
        //nothing
    }else{
        Make_color(VGA_LIGHT_RED, VGA_BLACK);
        WriteTerminal("Unknown Command: ");
        Make_color(VGA_WHITE, VGA_BLACK);
        WriteTerminal(command);
        WriteTerminal("\n");
    }

}