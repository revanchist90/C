#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "tree.h"

void child(int fd,struct tree_node *root)
{
	int final,i;
        int nbr_child=(root->nr_children);
        char *Name=(root->name);
        struct tree_node *childs;
        int status;
        pid_t pid;
        int nbr[nbr_child];
        int pfd[2];
       
        if(nbr_child==0){
          final=atoi(Name);
          printf("Process with PID = %ld is writing to pipe\n",(long)getpid());  
          if (write(fd, &final, sizeof(final)) != sizeof(final)){
		perror("Process: write to pipe");
		exit(1);
	                                                        }
          printf("Process with PID = %ld is exiting\n",(long)getpid());
          exit(1);
                        }
        else{
          printf("Process with PID = %ld is creating pipe\n",(long)getpid());
	  if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	                     }
          for(i=0;i<nbr_child;i++){
             childs=root->children+i;
             printf("Process with PID = %ld is creating child\n",(long)getpid());   
             pid=fork();
             if (pid < 0){
		perror("fork");
		exit(1);
	                 }
	     if (pid == 0){
		child(pfd[1],childs);
		assert(0);
	                  }
             printf("Process with PID = %ld is waiting for child with PID = %ld to terminate\n",(long)getpid(),(long)pid);   
             pid = wait(&status);
	     explain_wait_status(pid, status);
             printf("Parent: My PID is %ld. Receiving an int value from the child.\n",
		(long)getpid());
             if (read(pfd[0], &nbr[i], sizeof(nbr[i])) != sizeof(nbr[i])) {
		perror("Process: read from pipe");
		exit(1);
	                                                                  }
                                  }
          if(Name[0]=='*')
            final=nbr[0]*nbr[1];
          else
            final=nbr[0]+nbr[1];   
          printf("Process with PID = %ld is writing to pipe\n",(long)getpid());  
          if (write(fd, &final, sizeof(final)) != sizeof(final)){
		perror("Process: write to pipe");
		exit(1);
           	                                                        }
          printf("Process with PID = %ld is exiting\n",(long)getpid());
          exit(1);

            }

}

int main(int argc, char *argv[])
{
	pid_t p;
	int pfd[2];
	int status;
	int FinVal;
        struct tree_node *root;

        if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);
	
	printf("Initial process is creating pipe\n");
	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	}

	printf("Initial process is creating the root of the tree\n");
	p = fork();
	if (p < 0) {
		perror("fork");
		exit(1);
	}
	if (p == 0) {
		child(pfd[1],root);
		assert(0);
	}
	
	printf("InitialProcess: Created root with PID = %ld, waiting for it to terminate...\n",(long)p);
	p = wait(&status);
	explain_wait_status(p, status);
        
	if (read(pfd[0], &FinVal, sizeof(FinVal)) != sizeof(FinVal)) {
		perror("InitialProcess: read from pipe");
		exit(1);
	}
        else
                printf("The final value of the tree is:%d\n",FinVal);    

	return 0;
}
