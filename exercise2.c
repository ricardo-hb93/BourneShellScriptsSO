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
    int pipefd[2];
    int pipe_res;
    int pipe_cmd;
    int flag0;
    int flag1;
    char buffer[1000];

    char *executable_file;
    char *executable_args[MAX_ARGS];

    pipe_res = pipe(pipefd);

    if (pipe_res == -1){
      fprintf(stderr, "Failed to create pipe\n");
      exit(EXIT_FAILURE);
    }

    for (pipe_cmd = 1; pipe_cmd >= 0; pipe_cmd--) {

        pid = fork();
        if (pipe_cmd == 1)
          flag0 = pid;
        else
          flag1 = pid;

        if (pid == -1)
        {
            fprintf(stderr, "parent: error in fork\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0){
        close(pipe_cmd);
        dup2(pipefd[pipe_cmd], pipe_cmd);
        close(pipefd[pipe_cmd]);
          if (pipe_cmd == 1) {
            executable_file = "/usr/bin/cut";
            executable_args[0] = "my own name for cut";
            executable_args[1] = "-c1-4";
            executable_args[2] = NULL;
          } else {
            executable_file = "/usr/bin/nl";
            executable_args[0] = "my own name for nl";
            executable_args[1] = NULL;
          }

          execvp(executable_file, executable_args);

          fprintf(stderr, "child: error in exec\n");
          exit(EXIT_FAILURE);
        }
        else
        {

            fprintf(stderr, "parent %d: waiting for children %d\n", getpid(), pid);

            if (!pipe_cmd){
              while ((pid = waitpid(-1, &status, WUNTRACED | WCONTINUED)) != -1)
              {
                if (pid == flag0){
                  kill(flag1, SIGINT);
                }
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
        }
  }
    return (EXIT_SUCCESS);
}
