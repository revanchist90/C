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
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

struct queblc
 {
  int  id;
  int  pid;
  char *name;
  char *prio;
  struct queblc *next;    
 };

typedef struct queblc  block;

int i;
block *last=NULL;
block *p=NULL;
block *first=NULL;

static void
sched_HighPrio_task(int id)
{
      block *node=first;

      p=first;
      while((node!=NULL)&&(node->id!=id))
        {
          p=node;
          node=node->next;
        }
      if((node!=NULL)&&(strcmp(node->prio,"HIGH")!=0)&&(node!=first))
        {
          p->next=node->next;
          if(p->next==NULL)
            last=p;
          p=first;
          while((p->next!=NULL)&&(strcmp((p->next)->prio,"LOW")!=0))
            p=p->next;
          node->next=p->next;
          p->next=node;
          node->prio="HIGH";
        }
      else if(node==NULL)
        printf("Process with id=%d doesnt exist\n",id);
      else if(strcmp(node->prio,"HIGH")==0)
        printf("Process with id=%d is already HIGH\n",id);
      else
        node->prio="HIGH";
}

static void
sched_LowPrio_task(int id)
{
      block *node=first;

      p=first;
      while((node!=NULL)&&(node->id!=id))
        {
          p=node;
          node=node->next;
        }
      if((node!=NULL)&&(strcmp(node->prio,"LOW")!=0)&&(node!=first))
        {
          p->next=node->next;
          if(p->next==NULL)
            last=p;
          last->next=node;
          last=node;
          last->next=NULL;
          node->prio="LOW";
        }
      else if(node==NULL)
        printf("Process with id=%d doesnt exist\n",id);
      else if(strcmp(node->prio,"LOW")==0)  
        printf("Process with id=%d is already LOW\n",id);
      else
        node->prio="LOW";
}

/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{
      int j=0;
      
      p=first;
      while(p!=NULL)
        {
          printf("%d:Process with id=%d,PID=%d,Name=%s,Prio=%s\n",j,p->id,p->pid,p->name,p->prio);   
          p=p->next;
          j=j+1; 
        }
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{
      block *node=first;
      
      p=first;
      while((node!=NULL)&&(node->id!=id))
        {
          p=node;
          node=node->next;
        }
      if((node!=NULL)&&(node!=first))
        {
          p->next=node->next;
          if(p->next==NULL)
            last=p;
          kill(node->pid,SIGKILL);
          free(node);
        }
      else if(node==first)
        kill(node->pid,SIGKILL);
      else
        printf("Process with id=%d doesnt exist\n",id);
      return -ENOSYS;
}


/* Create a new task.  */
static void
sched_create_task(char *executable)
{
      int pid;

      printf("Creation of process with id:%d\n",i); 
      pid = fork();
      if(pid < 0) 
       {
         perror("main: fork");
	 exit(1);
       }
      if(pid == 0) 
       {
         printf("Process with id:%d,PID:%ld is starting to run\n",i,(long)getpid());
                 
	 char *newargv[] = { executable, NULL, NULL, NULL };
	 char *newenviron[] = { NULL };

         printf("Process with id:%d,PID=%ld suspend self...\n",i,(long)getpid());
         raise(SIGSTOP);
         printf("I am the process with id=%d,PID=%ld\n",i,(long)getpid());
	 printf("About to replace myself with the executable %s...\n",executable);
         execve(executable, newargv, newenviron);

	 /* execve() only returns on error */
	 perror("execve");
	 exit(1);
       }
           
      p=(block *)malloc(sizeof(block));
      p->id=i;
      p->pid=pid;
      p->name=executable;
      p->prio="LOW";
      last->next=p;
      last=p;
      last->next=NULL;
      i=i+1;
}

/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

                case REQ_HIGH_TASK:
                        sched_HighPrio_task(rq->task_arg);
                        return 0;

                case REQ_LOW_TASK:
                        sched_LowPrio_task(rq->task_arg);
                        return 0;

		default:
			return -ENOSYS;
	}
}

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
     block *node=NULL;
            
     do 
      {
	 pid=waitpid(-1, &status, WUNTRACED | WNOHANG);
	 if(pid < 0)
           {
	     perror("waitpid");
	     exit(1);
	   }
         if(pid!=0)
	   explain_wait_status(pid, status);
	 if((WIFEXITED(status) || WIFSIGNALED(status))&&(first->pid==pid))
	   {
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
	 if((WIFSTOPPED(status))&&(first->pid==pid)&&(strcmp(first->prio,"LOW")==0))
           {
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
         else if((WIFSTOPPED(status))&&(first->pid==pid)&&(strcmp(first->prio,"HIGH")==0))
           {
             p=first;
             node=first;
             while((node->next!=NULL)&&(strcmp((node->next)->prio,"LOW")!=0))
               node=node->next;
             if(node!=first)
               {
                 first=first->next;
                 p->next=node->next;
                 node->next=p;
                 if(p->next==NULL)
                   last=p;
               }
             alarm(SCHED_TQ_SEC);
             printf("Process with id=%d,PID=%d will start running\n",first->id,first->pid);
	     kill(first->pid,SIGCONT); 
           }           
      }while(pid > 0);
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
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

static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;
        
        printf("Shell with id=%d,PID=%ld suspend self...\n",0,(long)getpid());
	raise(SIGSTOP);
        printf("I am Shell,id=%d,PID=%ld\n",0,(long)getpid());
	printf("About to replace myself with the executable %s...\n",executable);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}
        
        printf("Creation of shell with id=%d\n",0);
	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
                printf("Shell with id=%d,PID=%ld is starting to run\n",0,(long)getpid());
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
       
        first=(block *)malloc(sizeof(block));
        first->id=0;
        first->pid=p;
        first->name=executable;
        first->prio="LOW";
        last=first;
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();
		ret = process_request(&rq);
		signals_enable();

		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int nproc;
        int pid;
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	/* TODO: add the shell to the scheduler's tasks */

	for(i=1;i<argc;i++)
         { 
           printf("Creation of process with id=%d\n",i); 
           pid = fork();
	   if(pid < 0) 
             {
              perror("main: fork");
	      exit(1);
	     }
	   if(pid == 0) 
             {
              printf("Process with id=%d,PID=%ld is starting to run\n",i,(long)getpid());
                 
              char executable[] = "prog";
	      char *newargv[] = { executable, NULL, NULL, NULL };
	      char *newenviron[] = { NULL };

              printf("Process with id=%d,PID=%ld suspend self...\n",i,(long)getpid());
              raise(SIGSTOP);
              printf("I am %s,id=%d,PID=%ld\n",argv[i],i,(long)getpid());
	      printf("About to replace myself with the executable %s...\n",executable);
              execve(executable, newargv, newenviron);

	      /* execve() only returns on error */
	      perror("execve");
	      exit(1);
	     }
           
           p=(block *)malloc(sizeof(block));
           p->id=i;
           p->pid=pid;
           p->name=argv[i];
           p->prio="LOW";
           last->next=p;
           last=p;
           last->next=NULL;   
                
         } 

	nproc = argc; /* number of proccesses goes here */

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
        kill(first->pid,SIGCONT);
	shell_request_loop(request_fd,return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
