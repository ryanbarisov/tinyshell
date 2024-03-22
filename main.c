#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void strip_newline(char * line)
{
	int pos = 0;
	while(line[pos] != '\0') ++pos;
	if(pos > 0) line[pos-1] = '\0';
}

int read_command(char * line, int bufsize)
{
	if(fgets(line, bufsize, stdin) == NULL)
		return 0;
	strip_newline(line);
	printf("Read command '%s'\n", line);
	return 1;
}

void execute_command(char * command)
{
	char * const args[] = {command, NULL};
	printf("Execute command '%s' without arguments\n", command);
	pid_t pid = fork();
	if(pid == -1)
		printf("Failed to fork; errno %d\n", errno);
	else if(pid != 0)
	{
		wait(NULL);
		fflush(stdout);
	}
	else
	{
		int result = execve(command, args, NULL);
		printf("command return %d errno %d\n", result, errno);
	}
}

int in_loop = 1;
void loop()
{
	char line[1024];
	do
	{
		fputs("shell> \0", stdout);
		if(!in_loop) break;
		if(read_command(line, 1024) == 0) break;
		execute_command(line);
	}
	while (1);
}

void sa_handler0(int sig)
{
	//printf("handle signal action for signal %d\n", sig);
	in_loop = 0;
}
void signal_handling()
{
	struct sigaction act;
	act.sa_handler = &sa_handler0;
	if(sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction errno %d\n", errno);
}

int main(int argc, char * argv[])
{
	signal_handling();
	printf("Start shell.\n");
	loop();
	printf("Finish shell.\n");
	return 0;
}
