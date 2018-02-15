#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void fork_procs(struct tree_node *root)
{	
        int status,i,j;
        char *Name=(root->name);
        int nbr_child=(root->nr_children);
        struct tree_node *child;
        pid_t pid[nbr_child];

	printf("PID = %ld, name %s, starting...\n",(long)getpid(),Name);
	change_pname(Name);

	if(nbr_child==0){
          printf("PID = %ld, name %s, Suspend self...\n",(long)getpid(),Name);
          raise(SIGSTOP);
          printf("PID = %ld, name %s, is awake...\n",(long)getpid(),Name);
          printf("PID = %ld, name %s, is exiting...\n",(long)getpid(),Name);
          exit(0);
                        }
        else
          for(i=0;i<nbr_child;i++){
            child=root->children+i;
            printf("PID = %ld, name %s, is creating child with name %s\n",(long)getpid(),Name,child->name);
            pid[i]=fork();
            if(pid[i] < 0){
		perror("main: fork");
		exit(1);
	               }
	    if(pid[i] == 0){
		fork_procs(child);
		exit(1);
	                }
                                  }
        
        printf("PID = %ld, name %s, checking if children raised SIGSTOP\n",(long)getpid(),Name);
        wait_for_ready_children(nbr_child);
        
        printf("PID = %ld, name %s, Suspend self...\n",(long)getpid(),Name);
	raise(SIGSTOP);
	printf("PID = %ld, name = %s is awake...\n",(long)getpid(),Name);
        
        for(j=0;j<nbr_child;j++){
           child=root->children+j; 
	   kill(pid[j], SIGCONT); 
           printf("PID = %ld, name = %s is waiting for child with PID = %ld, name %s to terminate\n",(long)getpid(),Name,(long)pid[j],child->name);
           wait(&status);
	   explain_wait_status(pid[j], status); 
                                }  
        
	printf("PID = %ld, name = %s is exiting...\n",(long)getpid(),Name);
	exit(0);
}

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);

	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		fork_procs(root);
		exit(1);
	}

	wait_for_ready_children(1);

	show_pstree(pid);

	kill(pid, SIGCONT);

	wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
