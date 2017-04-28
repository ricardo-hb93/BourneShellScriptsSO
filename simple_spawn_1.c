uthor: Guillermo Pérez Trabado <guille@ac.uma.es>
 *
 * Created on 24 de abril de 2017, 14:39
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 
#define MAX_ARGS 256
 
/*
 *
 */
 
void postman(int s){
   
    int status;
    pid_t pid;
    int code;
    int signum;
    int pipefd[2];
 
    char *executable_file;
    char *executable_args[MAX_ARGS];
 
char buffer[2000];
 
    while ((pid = waitpid(-1, &status, WUNTRACED | WCONTINUED)) != -1)
        {
            // Status is a multifield value with this information:
            //      Event: Type of state change (EXITED|TERMINATED BY SIGNAL|STOPPED|CONTINUED)
            //      Event information: Number of the signal which terminated/stopped the child
            //      Exit code for exited processes.
 
            // Use macros to check fields. NEVER USE status directly!!!
            if (WIFEXITED(status))
            {
                code = WEXITSTATUS(status);
                sprintf(buffer, "parent: child %d terminated with exit(%d)\n", pid, code);
            }
            if (WIFSIGNALED(status))
            {
                signum = WTERMSIG(status);
                sprintf(buffer, "parent: child %d kill by signal %d\n", pid, signum);
            }
            if (WIFSTOPPED(status))
            {
                signum = WSTOPSIG(status);
                sprintf(buffer, "parent: child %d stopped by signal %d\n", pid, signum);
            }
            if (WIFCONTINUED(status))
            {
                sprintf(buffer, "parent: child %d continued\n", pid);
            }
write(2,buffer,strlen(buffer));
        }
 
}
 
int main(int argc, char** argv)
{
    int status;
    pid_t pid;
    int code;
    int signum;
    int pipefd[2];
 
    char *executable_file;
    char *executable_args[MAX_ARGS];
 
    /***************************************************************************/
    /* This is the parent process before forking. Every change done is this section
     * will affect both the parent and the child (inherited).
     */
 
    // <PRE-FORK COMMON CODE HERE>
 
    /***************************************************************************/
 
    pid = fork();
 
    /* Errors in fork mean that there is no child process. Parent is alone. */
    if (pid == -1)
    {
        /* Error in fork: This is severe!!! Abort further processing!!! */
        fprintf(stderr, "parent: error in fork\n");
        exit(EXIT_FAILURE);
    }
 
    /* Fork has succeeded. Sentences after fork are being executed both by parent
     * and child but in their respective process contexts (isolated from each other).
     */
    if (pid == 0)
    {
        /***************************************************************************/
        /* This is the child process. Evey change in this section only affects the
         * child environment.
         */
 
        // <POST-FORK CHILD ONLY CODE HERE>
 
        /***************************************************************************/
 
        /* Here we can execute a new binary which will assume that the file descriptors
         * 0, 1 and 2 are already opened.
         */
        executable_file = "/bin/ps";
        executable_args[0] = "my own name for ps"; // A custom name for the process
        executable_args[1] = "l"; // First argument
        executable_args[2] = NULL; // Last argument
   
    char buffer [2000];
 
    sprintf(buffer, "child 2: %d %s\n", getpid(), executable_file);
        execvp(executable_file, executable_args);
 
        /* Error in exec: This is severe!!! Abort further processing in child!!!
         * Notify parent process through exit code!!!
         * Assign special return values to each kind of error.
         */
        fprintf(stderr, "child: error in exec\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        /***************************************************************************/
        /* This is the parent process after fork. Evey change in this section
         * only affects the parent environment.
         */
 
        // <POST-FORK PARENT ONLY CODE HERE>
        fprintf(stderr, "parent: waiting for children %d\n", getpid());
 
        // <PARENT MUST CLOSE DESCRPTORS FROM PIPES IN USE BY CHILDREN>
        close(pipefd[1]);
        close(pipefd[0]);
 
        signal(SIGCHLD,postman);
 
        while(1)
        {
                sleep(5);
                printf("time flies like arrows\n");
        }
        /***************************************************************************/
    }
 
    /* Only parent process should reach here */
    return (EXIT_SUCCESS);
}
