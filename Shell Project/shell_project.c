#include "shell_project.h"

job * job_control_list;

int main(void){
	char hostname[MAX_LINE];
	char command_entered[MAX_LINE];
	char *args[MAX_LINE/2];
	char previous_directory[MAX_LINE];
	char current_path[MAX_LINE];
	bool is_background;
	enum status status_res;
	int info;
	int status;
	int waitpid_return;
	pid_t pid_fork;

	ignore_terminal_signals();
	job_control_list = new_list("Job control list");
	signal(SIGCHLD, parent_handler);

	gethostname(hostname, MAX_LINE);
	struct passwd * pwd = getpwuid(getuid());

	strcpy(previous_directory, "not initializated");

	while (TRUE) {
		print_prompt(pwd->pw_name, pwd->pw_dir, hostname, current_path);
		get_command(command_entered, MAX_LINE, args, &is_background);
		if (is_empty(args, 0))
			continue;

		if (is_cd(command_entered)){
			change_directory(args, previous_directory, pwd->pw_dir);
			continue;
		} else if (!strcmp(command_entered, "jobs")){
			if (list_size(job_control_list))
				print_job_list(job_control_list);
			continue;
		} else if (!strcmp(command_entered, "fg")){
			if (list_size(job_control_list))
				foreground(args);
			else
				printf("fg: current: no such job\n");
			continue;
		} else if (!strcmp(command_entered, "bg")){
			if (list_size(job_control_list))
				background(args);
			else
				printf("bg: current: no such job\n");
			continue;
		}

		if ((pid_fork = fork()) == -1) {
			fprintf(stderr, "parent: error in fork\n");
			exit(1);
		}

		if (!pid_fork) {	// change by is_child(pid_fork)?
			child_creation(pid_fork, command_entered, args); // command entered is the same as args[0] ?
		} else {
			block_SIGCHLD();
			add_job(job_control_list, new_job(pid_fork, command_entered, is_background ? BACKGROUND : FOREGROUND));
			unblock_SIGCHLD();
			if (is_background) {
				report_background(pid_fork, command_entered);  // cambiar ?
			} else {
				set_terminal(pid_fork);
				if ((waitpid_return = waitpid(pid_fork, &status, WUNTRACED)) == -1) {
					fprintf(stderr, "parent: error in waitpid");
          exit(EXIT_FAILURE);
        }
				set_terminal(getpid());
				status_res = analyze_status(status, &info);
				if (status_res == SUSPENDED){
					block_SIGCHLD();
					get_item_bypid(job_control_list, pid_fork)->state = STOPPED;
					printf("\n");
					print_job_list(job_control_list);
					unblock_SIGCHLD();
				} else {
					block_SIGCHLD();
					report_foreground(pid_fork, command_entered, info, status_res);
					delete_job(job_control_list, get_item_bypid(job_control_list, pid_fork));
					unblock_SIGCHLD();
				}
			}
		}
	}
}

void background(char ** args){
	int elem;
	job * current_job;

	if (args[1] == NULL || args[1][0] == '\0'){
		elem = 1;
	} else {
		elem = atoi(args[1]);
		elem = elem == 0? 1 : elem;
		if (elem > (list_size(job_control_list))){
			printf("fg: %d: no such job\n", elem);
			return;
		}
	}
	current_job = get_item_bypos(job_control_list, elem);
//	printf("%d\n", current_job->state);
	if (current_job->state == STOPPED){
//		printf("%s\n", current_job->command);
		current_job->state = BACKGROUND;
		killpg(current_job->pgid, SIGCONT);
		report_background(current_job->pgid, current_job->command);
	}
}

void foreground(char ** args){
	int status, info, waitpid_return, elem;
	enum status status_res;
	job * current_job;

	if (args[1] == NULL || args[1][0] == '\0')
		elem = 1;
	else {
		elem = atoi(args[1]);
	//	printf("%d\n", elem);
		elem = elem == 0? 1 : elem;
		if (elem > (list_size(job_control_list))){
			printf("fg: %d: no such job\n", elem);
			return;
		}
	}
	current_job = get_item_bypos(job_control_list, elem);
	printf("%s\n", current_job->command);
	killpg(current_job->pgid, SIGCONT);
	set_terminal(current_job->pgid);
	if ((waitpid_return = waitpid(current_job->pgid, &status, WUNTRACED)) == -1) {
		fprintf(stderr, "parent: error in waitpid");
		exit(EXIT_FAILURE);
	}
	status_res = analyze_status(status, &info);
//	printf("%d\n", status_res);
	if (status_res == EXITED || status_res == SIGNALED){
		block_SIGCHLD();
		report_foreground(current_job->pgid, current_job->command, info, status_res);
		delete_job(job_control_list, get_item_bypid(job_control_list, current_job->pgid));
		unblock_SIGCHLD();
	}
	set_terminal(getpid());
}

void parent_handler(int sig){
	block_SIGCHLD();

	int i = 1;
	int status;
	int wait_res;
	int info;
	enum status status_res;
	char message[MAX_LINE];
	job * current_job;

	while (i <= list_size(job_control_list)){
		current_job = get_item_bypos(job_control_list, i);
		wait_res = waitpid(current_job->pgid, &status, WNOHANG | WUNTRACED);
		if (wait_res == current_job->pgid){
			status_res = analyze_status(status, &info);
			sprintf(message, "Info: %s%s pid: %d, command: %s, %s, info: %d\n%s",
				CYAN, state_strings[current_job->state], current_job->pgid, current_job->command, status_strings[status_res], info, WHITE);
			write(STDOUT_FILENO, "\n", 1);
			write(STDOUT_FILENO, message, strlen(message));
			if (status_res == SUSPENDED){
				current_job->state = STOPPED;
				i++;
			} else {
				delete_job(job_control_list, current_job);
			}
		} else {
			status_res = analyze_status(status, &info);
			if (status_res == 2){
				current_job->state = STOPPED;
			}
			i++;
		}
	}

	unblock_SIGCHLD();
}

void print_prompt(char * username, char * homedir_name, char * hostname, char current_path[MAX_LINE]){
	getcwd(current_path, MAX_LINE);
	if_at_home_change_current_path(current_path, homedir_name);
	printf(YELLOW BOLD "%s@%s" WHITE NORMAL ":" BLUE BOLD "%s" WHITE NORMAL "$ ", username, hostname, current_path);
	fflush(stdout);
}

void report_background(pid_t pid_fork, char command_entered[MAX_LINE]){
        printf("Info: %s%s job running... pid: %d, command: %s\n%s", MAGENTA, state_strings[1], pid_fork, command_entered, WHITE);
}

void report_error(char command_entered[MAX_LINE]){
	printf("Info: %sError, command not found: %s\n%s", RED, command_entered, WHITE);
}

void report_foreground(pid_t pid_fork, char command_entered[MAX_LINE], int info, int status){
	printf("\nInfo: %s%s pid: %d, command: %s, %s, info: %d\n%s", CYAN, state_strings[0], pid_fork, command_entered, status_strings[status], info, WHITE);
}

bool is_empty(char *args[MAX_LINE/2], int i){
	return args[i] == NULL;
}

bool is_cd(char command_entered[MAX_LINE]){
	return !(strcmp("cd", command_entered));
}

void if_at_home_change_current_path(char current_path[MAX_LINE], char * homedir_name){
	char * home_starts_here;
	home_starts_here = strstr(current_path, homedir_name);
	if (home_starts_here != NULL){
		strcpy(current_path, (home_starts_here-1 + strlen(homedir_name)));
		current_path[0] = '~';
	}
}

void child_creation(pid_t pid_fork, char command_entered[MAX_LINE], char *args[MAX_LINE/2]){
	signal(SIGCHLD, SIG_DFL);
	pid_fork = getpid();
	if (new_process_group(pid_fork))
		fprintf(stderr, "child: error in new_process_group\n");
	restore_terminal_signals();
	execvp(command_entered, args);
	report_error(command_entered);
	exit(-1);
}

void change_directory(char *args[MAX_LINE/2], char previous_directory[MAX_LINE], char * homedir_name){
	char new_directory[MAX_LINE/2];
	char current_directory[MAX_LINE];
	char first_character;
	char second_character;
	char third_character;
	bool change_directory = TRUE;
	bool failed_cd;
	getcwd(current_directory, MAX_LINE);
	if (is_empty(args, 1))
		strcpy(new_directory, homedir_name);
	else {
		first_character = args[1][0];
		second_character = args[1][1];
		third_character = args[1][2];

		switch (first_character) {
			case '-':
				if (second_character == '\0'){
					if (!strcmp(previous_directory, "not initializated")) {
						printf("info: cd: previous_directory not set\n");
						change_directory = FALSE;
					} else {
						strcpy(new_directory, previous_directory);
						printf("%s\n", new_directory);
					}
				} else if (second_character == '-' && third_character == '\0'){
					strcpy(new_directory, homedir_name);
				} else {
					printf("info: cd: --: invalid option\n");
					change_directory = FALSE;
				}
				break;
			case '~':
				strcpy(new_directory, homedir_name);
				strcat(new_directory, args[1]+1);
				break;
			default:
				strcpy(new_directory, args[1]);
				break;
		}
	}
	if (change_directory){
	getcwd(previous_directory, MAX_LINE);
	failed_cd = chdir(new_directory);
	if (new_directory[0] == '\0')
		failed_cd = 0;
	if (failed_cd) {
		printf("Info cd: %s: No such file or directory\n", args[1]);
		chdir(current_directory);
	} else {
		printf("Info: %sUsed cd (change directory)\n", CYAN);
	}
	}
}
