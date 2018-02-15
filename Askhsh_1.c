#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */

struct queblc
 {
  int  id;
  int  pid;
  struct queblc *next;    
 };

typedef struct queblc  block;

block *last=NULL;
block *p=NULL;
block *first=NULL;
 
/* SIGALRM handler: Gets called whenever an alarm goes off.
 * The time quantum of the currently executing process has expired,
 * so send it a SIGSTOP. The SIGCHLD handler will take care of
 * activating the next in line.
 */
static void
sigalrm_handler(int signum)
{
	printf("Process with PID:%d suspend self...\n",first->pid);
        kill(first->pid,SIGSTOP);          
}

/* SIGCHLD handler: Gets called whenever a process is stopped,
 * terminated due to a signal, or exits gracefully.
 *
 * If the currently executing task has been stopped,
 * it means its time quantum has expired and a new one has
 * to be activated.
 */
static void
sigchld_handler(int signum)
{
     int pid;
     int status;
            
     do 
      {
	 pid=waitpid(-1, &status, WUNTRACED | WNOHANG);
	 if(pid < 0)
           {
	     perror("waitpid");
	     exit(1);
	   }
	 if((WIFEXITED(status) || WIFSIGNALED(status))&&(first->pid==pid))
	   {
             explain_wait_status(pid, status);
             p=first; 
             first=first->next;
             free(p);
             if(first==NULL)
               {
                 printf("no tasks remaining exiting\n");
                 exit(1);
               }
             else
               {
                 alarm(SCHED_TQ_SEC);
                 printf("Process with id=%d,PID=%d will start running\n",first->id,first->pid);
                 kill(first->pid,SIGCONT);
               }               
           }
	 if((WIFSTOPPED(status))&&(first->pid==pid))
           {
             explain_wait_status(pid, status);
             p=first;
             if(first!=last)
               {
                 first=first->next;
                 last->next=p;
                 last=p;
                 last->next=NULL;
               }
             alarm(SCHED_TQ_SEC);
             printf("Process with id=%d,PID=%d will start running\n",first->id,first->pid);
	     kill(first->pid,SIGCONT); 
           }           
      }while(pid > 0);
}


/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}




int main(int argc, char *argv[])
{
	int nproc;
        int pid;
        int i;
        
        for(i=1;i<argc;i++)
         { 
           printf("Creation of process with id:%d\n",i); 
           pid = fork();
	   if (pid < 0) 
               {
		perror("main: fork");
		exit(1);
	       }
	   if (pid == 0) 
               {
                printf("Process with id:%d,PID:%ld is starting to run\n",i,(long)getpid());
                 
		char executable[] = "prog";
		char *newargv[] = { executable, NULL, NULL, NULL };
		char *newenviron[] = { NULL };

                printf("Process with id=%d,PID=%ld suspend self...\n",i,(long)getpid());
                raise(SIGSTOP);
                printf("I am %s, PID = %ld\n",argv[i], (long)getpid());
	        printf("About to replace myself with the executable %s...\n",executable);
                execve(executable, newargv, newenviron);

	        /* execve() only returns on error */
	        perror("execve");
	        exit(1);
	       }
           
        	p=(block *)malloc(sizeof(block));
        	p->id=i;
                p->pid=pid;
        	if (first==NULL)
            	{
              		first=p;
              		first->next=NULL;
              		last=first;    
                }
                else
            	{
                	last->next=p;
              		last=p;
              		last->next=NULL;   
                }
         } 
	nproc = argc-1; /* number of proccesses goes here */
        
	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);
        
	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}

        alarm(SCHED_TQ_SEC);
        printf("Process with id=%d,PID=%d will start running\n",first->id,first->pid);
        kill(first->pid, SIGCONT);
	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
