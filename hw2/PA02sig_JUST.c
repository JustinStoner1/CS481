#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static int run = 1;
static int childToggle = 1;//1 = on, 0 = off
static pid_t pid;

void ctrlC(int c)
{
	printf("\ncaught ctrl-c, ending child process and terminating\n");
    run = 0;
	kill(pid, SIGKILL);
	//kill(pid, SIGKILL);
}

void ctrlZ(int z)
{
	printf("\ncaught ctrl-z, ");
	childToggle = !childToggle;
	if (childToggle)
	{		
		printf("child will resume\n");
		kill(pid, SIGCONT);
	}
	else
	{
		printf("child is suspended\n");
		kill(pid, SIGTSTP);
	}
}

int main(void)
{

	struct sigaction ctrlZS;
	ctrlZS.sa_handler = ctrlZ;
	struct sigaction ctrlCS;
	ctrlCS.sa_handler = ctrlC;
	//catch signals

	sigaction(SIGTSTP, &ctrlZS, NULL);
	sigaction(SIGINT, &ctrlCS, NULL);

	//signal(SIGTSTP, ctrlZ);
	//signal(SIGINT, ctrlC);

	if ((pid = fork()) == 0)//make child
	{
		printf("child has been created\n");
		char *args[] = {"y",NULL};
		char *env[] = {0,NULL};
		execve("/usr/bin/yes", args, env);
	}
	else if (pid < 0)//catch fork errors
	{
		printf("fork error");
	}
	else
	{
		printf("running main program\n");
		//int c = 0;
		while (run)
		{
			//c++;
			//printf("p[%d]\n",c);
			sleep(1);
		}
		printf("waiting for child to terminate\n");
		wait(NULL);
		printf("child has terminated, terminating main program\n");
	}
}

