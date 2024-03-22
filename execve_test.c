#include <stdio.h>
#include <unistd.h> // execve
#include <errno.h>

int main(int argc, char ** argv, char ** envp)
{
	const char * pathname = "/usr/bin/ls";
	//const char * pathname = "./script.sh";
	char * const args[] = {pathname, "-lrt", NULL};
	printf("executing command: %s with argument %s\n", pathname, args[1]);
	int result = execve(pathname, args, NULL);
	printf("command return %d errno %d\n", result, errno);
	return 0;
}
