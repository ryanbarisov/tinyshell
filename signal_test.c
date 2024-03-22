#include <stdio.h>
#include <signal.h>
#include <errno.h>

int in_loop = 1;


void sa_handler0(int sig)
{
	//printf("handle signal action for signal %d\n", sig);
	in_loop = 0;
}


int main(int argc, char ** argv)
{
	printf("Echoing lines from stdin until Ctrl-C or Ctrl-D is not pressed.\n");
	char line[1024];
	struct sigaction act;
	act.sa_handler = &sa_handler0;
	if(sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction errno %d\n", errno);
	do
	{
		fputs("shell> \0", stdout);
		if(!in_loop) break;
		if(fgets(line, 1024, stdin) == NULL) break;
		fputs(line, stdout);
		fflush(stdout);
	}
	while (1);
	printf("\r\nFinish program.\n");
	return 0;
}
