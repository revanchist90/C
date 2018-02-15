#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void fork_procs(void)
{

        pid_t pid;
	int status;

	change_pname("A");

        printf("Process 'A' creates process 'B'\n");
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	             }
	if (pid == 0){
                printf("Process 'B' is starting to run\n");
		change_pname("B");
                printf("Process 'B' creates process 'D'\n");
                pid = fork();
                if (pid < 0) {
		perror("fork");
		exit(1);
	                     }
	        if (pid == 0){
                        printf("Process 'D' is starting to run\n");
                        change_pname("D");
                        printf("Process 'D' is sleeping\n");
	                sleep(SLEEP_PROC_SEC);
                        printf("Process 'D' is exiting\n");
                        exit(13);
                             }
                printf("Process 'B' is waiting for child to terminate\n");
                pid = wait(&status);
	        explain_wait_status(pid, status);
                printf("Process 'B' is exiting\n");
                exit(19);
                      }

        printf("Process 'A' creates process 'C'\n");
        pid = fork();
        if (pid < 0){
                perror("fork");
		exit(1);
                    }
        if (pid == 0){
                printf("Process 'C' is starting to run\n");
                change_pname("C");
                printf("Process 'C' is sleeping\n");
	        sleep(SLEEP_PROC_SEC);
                printf("Process 'C' is exiting\n");
                exit(17);
                     }

        printf("Process 'A' is waiting for child to terminate\n"); 
        pid = wait(&status);
	explain_wait_status(pid, status);
        
        printf("Process 'A' is waiting for child to terminate\n");
        pid = wait(&status);
	explain_wait_status(pid, status); 
        
        printf("Process 'A' is exiting\n");
	exit(16);
}

int main(void)
{
	pid_t pid;
	int status;

        printf("The initial process creates the root of the tree,\nits called 'A'\n");
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
                printf("Process 'A' is starting to run\n");
		fork_procs();
		exit(1);
	}

	sleep(SLEEP_TREE_SEC);
	show_pstree(pid);

	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
