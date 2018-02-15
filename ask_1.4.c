#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3


void fork_procs(struct tree_node *root)
{
	
	        printf("PID = %ld, name %s, starting...\n",
			(long)getpid(), root->name);
	        change_pname(root->name);
	
	
           int value;
           int pfd[2];
            pid_t p;
            if (pipe(pfd) < 0) {
             perror(“pipe”);
            exit(1);
              }
            p = fork();
            if (p < 0) {
            perror(“fork”);
             exit(1);
            } 
            else if (p == 0) {
            if (read(pfd[0], &value, sizeof(value)) != sizeof(value)) {
             perror(“read from pipe”);
              exit(1);
            }
            printf(“child received value: value = %f\n”, value);
            exit(0);
          } 
          else {
           compute_value(&value);
            if (write(pfd[1], &value, sizeof(value)) != sizeof(value))
             {
            perror(“write to pipe”);
             exit(1);
           }
         exit(0);
               }
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}

	/* Read tree into memory */
	root = get_tree_from_file(argv[1]);

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs(root);
		exit(1);
	}

	/*
	 * Father
	 */
	/* for ask2-signals */
	//wait_for_ready_children(1);

	/* for ask2-{fork, tree} */
      sleep(SLEEP_TREE_SEC);  
	/* Print the process tree root at pid */
	show_pstree(pid);

	/* for ask2-signals */
	//kill(pid, SIGCONT);

	/* Wait for the root of the process tree to terminate */
	wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
