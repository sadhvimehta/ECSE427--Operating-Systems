#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>
#include <sys/time.h>
#include <time.h>

double getTime() { // method to measure time of implementations

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	double i = tp.tv_sec * 1000000.0;
	i = i + tp.tv_nsec/1000.0;
	return i;
}

int my_system(char* command) {
	double timerStart, timerEnd;
	timerStart = getTime();

	// below, parsing input command for execvp function's arguments
	char *var = strchr(command, '\n');
	if(var != NULL) {*var = '\0';} // removing unneccessary newline characters for wordexp to work
	
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

	int childPid = fork(); // return value of fork in respective processes

	if(childPid < 0) {
		printf("Failed to fork process. Please try entering the command again\n");
		fflush(stdout);
		return -1;
	}
	else if (childPid == 0) { // in child process
		execvp(result.we_wordv[0], result.we_wordv); // executing input command
		_exit(127); // execvp did not terminate successfully, exit shell with status 127
	}
	else {	// in parent process
		int status;
		wait(&status); // wait for child process to terminate
		if(WIFEXITED(status)) { // handles error message for when execvp has unsuccessful termination
			int exitStatus = WEXITSTATUS(status);
			if(exitStatus == 127) {
				printf("sh: 1: %s: not found\n", result.we_wordv[0]);
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
