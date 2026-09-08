#include "keyboard.h"
#include "io.h"
#include "terminal.h"
#include "interrupts.h"
#include "shell.h"
#include "task.h"

#define KEY_QUEUE_SIZE 128

static char key_queue[KEY_QUEUE_SIZE];
static unsigned int key_head = 0;
static unsigned int key_tail = 0;

static int shift_pressed = 0;


static char input_buffer[INPUT_SIZE];
static unsigned int input_pos = 0;

static Task *keyboard_waiting_task = 0;
static int user_input_mode = 0;

void KeyboardSetUserMode(int enabled){
    user_input_mode = enabled;
}

void KeyboardClearQueue(void)
{
    key_head = 0;
    key_tail = 0;
}

void KeyboardWaitForInput(void){
    keyboard_waiting_task = GetCurrentTask();
}

static void QueueKey(char c)
{
    unsigned int next =
        (key_head + 1) % KEY_QUEUE_SIZE;

    if (next == key_tail) {
        return;
    }

    key_queue[key_head] = c;
    key_head = next;
    if(keyboard_waiting_task != 0){
        WakeTask(keyboard_waiting_task);
        keyboard_waiting_task = 0;
    }
}

char KeyboardReadChar(void)
{
    if (key_head == key_tail) {
        return 0;
    }

    char c = key_queue[key_tail];

    key_tail =
        (key_tail + 1) % KEY_QUEUE_SIZE;

    return c;
}
void keyboard_handler(void)
{
    unsigned char scancode = inb(0x60);
    char c;


        if(scancode ==0x2A || scancode == 0x36){
            shift_pressed = 1;
        }else if(scancode == 0xAA || scancode == 0xB6){
            shift_pressed = 0;
        }else{
            if (!(scancode & 0x80)) {
                if(shift_pressed){
                    c = scancode_to_ascii_shift[scancode];
                }else{
                    c = scancode_to_ascii[scancode];
                }

                if(c != 0){
                    if(user_input_mode){
                        QueueKey(c);
                        return;
                    }
                    
                    
                    if(c == '\b' ){
                        if(input_pos > 0){
                            input_pos--;
                            input_buffer[input_pos ] = '\0';
                            terminal_putchar(c);
                        }
                    }else if(c == '\n'){

                        input_buffer[input_pos] = '\0';
                        NewLine();
                        ProcessCommand(input_buffer);

                        input_pos = 0;
                        input_buffer[0] = '\0';

                        if (!user_input_mode) {
                            PrintPrompt();
                        }
                    }else
                    {
                        if(input_pos < INPUT_SIZE -1){
                        input_buffer[input_pos]= c;
                        input_pos++;
                        input_buffer[input_pos] = '\0';
                        terminal_putchar(c);

                        }         
                    }
                }


                
            }

        }
}

