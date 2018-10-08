#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>
#include <sys/time.h>
#include <time.h>

#define STACK_SIZE (1024*1024) // stack size for cloned child

double getTime() { // method to measure time of implementations

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	double i = tp.tv_sec * 1000000.0;
	i = i + tp.tv_nsec/1000.0;
	return i;
}

int clone_function(void* input) {

	if(strcmp(*((char **) input), "cd") == 0 ) { // check if input command is a 'cd' or not
		int status = chdir(*((char **) input + 1));
		if(status < 0) { // check if 'cd' command executed successfully or not
			exit(1);
		}
		exit(0);
	}
	else {
		execvp(*((char **) input), (char**) input); // executing input command
		// execvp did not terminate successfully
		exit(127);
	}

}

int my_system(char* command) {
	double timerStart, timerEnd;
	timerStart = getTime();

	char *stack; // start of child stack buffer
	char *stackTop; // end of child stack buffer
	pid_t childPid; // stores returned value of child process

	stack = malloc(STACK_SIZE);
	if (stack == NULL) {
		perror("Failed to allocate memory for child process. Please try again\n");
		return -1;
	}
	
	stackTop = stack + STACK_SIZE; // do this because stack grows downwards

	// below, parsing input command for execvp function's arguments
	char *var = strchr(command, '\n');
	if(var != NULL) {*var = '\0';} // remove unneccessary newline characters for wordexp to work
	
	wordexp_t result;
	switch(wordexp(command, &result, 0)) {
		case 0:
			break;
		case WRDE_NOSPACE: // attempt to allocate memory failed
			wordfree(&result);
			break;
		default:
			return -1;
	}
	
	childPid = clone(clone_function, stackTop, CLONE_VFORK | CLONE_FS | SIGCHLD, result.we_wordv); 

	if(childPid < 0) {
		printf("Failed to clone process. Please try entering the command again\n");
		fflush(stdout);
		free(stack);
		return -1;
	}
	else {	// in parent process
		int status;
		waitpid(childPid, &status, 0); 

		if(WIFEXITED(status)) { // handles error message for when execvp has unsuccessful termination
			int exitStatus = WEXITSTATUS(status);
			if(exitStatus == 127) {
				printf("sh: 1: %s: not found\n", result.we_wordv[0]);
				fflush(stdout);
			}
			else if(exitStatus == 1) {
				printf("no such file/directory\n");
				fflush(stdout);
			}
		}
		else if(WIFSIGNALED(status)){ // handles display of signal message when child process is terminated by a signal
			int signal = WTERMSIG(status);
			char *signalMessage = strsignal(signal);
			if( signalMessage != NULL){
				printf("%s\n", signalMessage);
				fflush(stdout);
			}
		}

		free(stack);
		wordfree(&result);

		timerEnd = getTime();
		printf("Total execution time: %f microseconds\n", (timerEnd - timerStart));
		fflush(stdout);
		return 0;
	}
}

int main(int argc, char *argv[]) {

	while(1) {
		char* input = (char*) malloc(512 * sizeof(char)); // input line cannot exceed 512 characters
		
		if(input == NULL) { // check to see if memory was dynamically allocated or not.
			printf("Failed to allocate memory. Please try entering the command again\n");
			fflush(stdout);
		}
		else {
			if(fgets(input, 512 * sizeof(char), stdin) != NULL && strlen(input) > 1) {
				my_system(input);
			}
			else {	
				return EXIT_SUCCESS;
			}
		}
		
		free(input);
	}

	return EXIT_SUCCESS;
}
