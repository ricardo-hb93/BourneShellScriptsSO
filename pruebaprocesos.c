#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

main(){
	int 	fd[2], nbytes;
	pid_t 	childpid;
	char string[] = "Hello, world!\n";
	char readbuffer[256];
	
	nbytes = strlen(string);
	printf("%s, %d\n", string, nbytes);

	pipe(fd);
	childpid = fork();

	if (childpid == -1){
		perror("fork");
		exit(1);
	}

	if (childpid == 0){
		/* Child process closes up input side of pipe */
		close(fd[0]);

		/* Send "string" through the output side of pipe */
		write(fd[1], string, (strlen(string)+1));
		exit(0);
	} else {
		/* Parent process closes up output side of pipe */
		close(fd[1]);
		
		wait();
		/* Read in a string from the input side of pipe */
		nbytes = read(fd[0], readbuffer, strlen(readbuffer));
		printf("Received string: %s\n", readbuffer);
	}
}
