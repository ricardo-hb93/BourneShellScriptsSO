#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ARGS 256

int main(int argc, char** argv)
{
    int status;
    pid_t pid;
    int code;
    int signum;

    char *executable_file;
    char *executable_args[MAX_ARGS];

    char *input_file_name = "/etc/services";
    char *output_file_name = "./output.txt";

    int input_file;
    int output_file;

    pid = fork();

    if (pid == -1)
    {
        fprintf(stderr, "parent: error in fork\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {

      input_file = open(input_file_name, O_RDONLY);
      close(0);
      dup2(input_file, 0);
      close(input_file);

      output_file = open(output_file_name, O_WRONLY);
      close(1);
      dup2(output_file, 1);
      close(output_file);


        executable_file = "/usr/bin/sort";
        executable_args[0] = "my own name for sort";
        executable_args[1] = NULL;

        execvp(executable_file, executable_args);

        fprintf(stderr, "child: error in exec\n");
        exit(EXIT_FAILURE);
    }
    else
    {
      fprintf(stderr, "parent: %d waiting for children %d\n", getpid(), pid);

        while ((pid = waitpid(-1, &status, WUNTRACED | WCONTINUED)) != -1)
        {
            if (WIFEXITED(status))
            {
                code = WEXITSTATUS(status);
                fprintf(stderr, "parent: child %d terminated with exit(%d)\n", pid, code);
            }
            if (WIFSIGNALED(status))
            {
                signum = WTERMSIG(status);
                fprintf(stderr, "parent: child %d kill by signal %d\n", pid, signum);
            }
            if (WIFSTOPPED(status))
            {
                signum = WSTOPSIG(status);
                fprintf(stderr, "parent: child %d stopped by signal %d\n", pid, signum);
            }
            if (WIFCONTINUED(status))
            {
                fprintf(stderr, "parent: child %d continued\n", pid);
            }
        }
    }
    return (EXIT_SUCCESS);
}
