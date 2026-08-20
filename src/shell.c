#include "terminal.h"
#include "interrupts.h"
#include "shell.h"
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
        Make_color(VGA_LIGHT_BLUE, VGA_BLACK);
        WriteTerminal("Available Commands:\nhelp  - Show this help\nclear  - Clear the screen\nticks  - Show timer ticks\necho <msg>  - Echo a message\n");
        Make_color(VGA_WHITE, VGA_BLACK);

    }else if(StringEquals(argv[0], "clear")){
        clear_terminal();

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
        Make_color(VGA_LIGHT_RED, VGA_BLACK);
        WriteTerminal("Unknown Command: ");
        Make_color(VGA_WHITE, VGA_BLACK);
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