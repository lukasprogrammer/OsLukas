#ifndef SHELL_H
#define SHELL_H


int StringEquals(const char *a, const char *b);
void ProcessCommand(const char *command);
int ParseCommand(char *input, char *argv[], int max_args);
#endif