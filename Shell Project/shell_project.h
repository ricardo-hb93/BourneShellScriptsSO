#define NORMAL  "\x1B[0m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define WHITE  "\x1B[37m"
#define BOLD  "\x1B[1m"

#include "job_control.h"
#include <string.h>

typedef int bool;
#define TRUE 1
#define FALSE 0
#define MAX_LINE 256

void print_prompt(char * username, char * homedir_name, char * hostname, char current_path[MAX_LINE]);
void report_background(pid_t pid_fork, char command_entered[MAX_LINE]);
void report_error(char command_entered[MAX_LINE]);
void report_foreground(pid_t pid_fork, char command_entered[MAX_LINE], int info, int status);
bool is_empty(char *args[MAX_LINE/2], int index);
void if_at_home_change_current_path(char current_path[MAX_LINE], char * homedir_name);
bool is_cd(char command_entered[MAX_LINE]);
void change_directory(char *args[MAX_LINE/2], char previous_directory[MAX_LINE], char * homedir_name);
void child_creation(pid_t pid_fork, char command_entered[MAX_LINE], char *args[MAX_LINE/2]);
void parent_handler(int sig);
void foreground(char ** args);
void background(char ** args);
