#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


void onSegFault()
{
    printf("Got SIGSEGV.. Will try to launch GDB\n");
    int mypid=getpid();
    printf("My PID=%d\n",mypid);
    int pid;
    pid=fork();
    if (pid<0)
    {
	printf("Unable to fork()\n");
	exit(1);
    }
    if (pid>0) // Parent process
    {
	int status;
//	waitpid(pid, &status, WEXITED);
	sleep(160000);
	printf("Exiting\n");
	exit(1);
    }
    else
    {
	// Child pid
	printf("Launching gdb...");
	const char args[10];
	char s[256];
	sprintf(s,"gdb --pid=%d --batch -ex \"thread apply all bt\"",mypid);
	system(s);
	kill(mypid,SIGKILL);
	exit(0);
    }
}

void initSegFaultHandler()
{
    struct sigaction act;
    memset(&act,0,sizeof(act));
    act.sa_handler=onSegFault;
    act.sa_flags=SA_RESTART;
    sigaction(SIGSEGV, &act, NULL);
}

void doSomething()
{
    char *arr=0;
    arr[0]=1;
}
