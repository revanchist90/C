#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void process_tree(struct tree_node *root){
     
     pid_t pid;
     int status,i,j;
     char *Name=(root->name);
     int nbr_child=(root->nr_children);
     struct tree_node *child;         

     printf("%s:starting\n",Name);
     change_pname(Name);
         
     if(nbr_child==0){
        printf("%s:sleeping\n",Name);
        sleep(SLEEP_PROC_SEC);
        printf("%s:exiting\n",Name);
        exit(1);
                     }
     else
        for(i=0;i<nbr_child;i++){
            child=root->children+i;
            printf("%s:creates %s\n",Name,child->name);
            pid=fork();
            if(pid < 0){
		perror("main: fork");
		exit(1);
	               }
	    if(pid == 0){
		process_tree(child);
		exit(1);
	                }
                                }
     
     for(j=0;j<nbr_child;j++){
        printf("%s:waiting for child to terminate\n",Name);
        pid = wait(&status);
	explain_wait_status(pid, status);
                             }
     
     printf("%s:exiting\n",Name);
     exit(1);

                                             }


int main(int argc, char *argv[])
{
	struct tree_node *root;
        pid_t pid;
	int status; 

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);
	print_tree(root);

        printf("The initial process creates the root of the tree\n");
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		process_tree(root);
		exit(1);
	}

	sleep(SLEEP_TREE_SEC);
	show_pstree(pid);

	pid = wait(&status);
	explain_wait_status(pid, status);


	return 0;
}
