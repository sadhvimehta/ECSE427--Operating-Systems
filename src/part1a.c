#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

double getTime() { // method to measure time of implementations

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	double i = tp.tv_sec * 1000000.0;
	i = i + tp.tv_nsec/1000.0;
	return i;
}

int main(int argc, char *argv[]) {
	double timerStart, timerEnd;

	int index = 0;
	while(1) {
		char* input = (char*) malloc(512 * sizeof(char)); // assuming each line won't exceed 512 characters
		
		if(input == NULL) { // check to see if memory was dynamically allocated or not 
			printf("Failed to allocate memory. Please try entering the command again\n");
			fflush(stdout);
		}

		else {
			if(fgets(input, 512 * sizeof(char), stdin) != NULL && strlen(input) > 1) {
				timerStart = getTime(); 
				system(input);
				timerEnd = getTime();
				printf("Total execution time: %f\n", (timerEnd - timerStart));
				fflush(stdout);
			}
			else {
				return EXIT_SUCCESS;
			}
		}
		
		free(input);
	}

	return EXIT_SUCCESS;
}
