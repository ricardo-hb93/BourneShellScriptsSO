#define NORMAL    "\x1B[0m"
#define RED       "\x1B[31m"
#define GREEN     "\x1B[32m"
#define YELLOW    "\x1B[33m"
#define BLUE      "\x1B[34m"
#define MAGENTA   "\x1B[35m"
#define CYAN      "\x1B[36m"
#define WHITE     "\x1B[37m"
#define BOLD      "\x1B[1m"

#include "job_control.h"
#include <string.h>

typedef int bool;
#define TRUE 1
#define FALSE 0
#define MAX_LINE 256

bool internal_command(char ** args, char * previous_directory, struct passwd * pwd);
void print_prompt(char * username, char * homedir_name, char * hostname, char * current_path);
void report_background(pid_t pid_fork, char * command_entered);
void report_error(char * command_entered);
void report_foreground(pid_t pid_fork, char * command_entered, int info, int status);
void if_at_home_change_current_path(char * current_path, char * homedir_name);
void change_directory(char ** args, char * previous_directory, char * homedir_name);
void child_creation(pid_t pid_fork, char ** args);
void parent_handler(int sig);
void foreground(char ** args);
void background(char ** args);
