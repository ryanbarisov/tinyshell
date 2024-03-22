#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

extern char ** environ;

int in_loop = 1;

// returns amount of tokens parsed
int parse_line(char * line, char ** tokens, int max_tokens)
{
	int i = 0;
	const char* delim = " \t\n";
	tokens[i] = strtok(line, delim);
	while(tokens[i] != NULL && i < max_tokens-1)
	{
		tokens[++i] = strtok(NULL, delim);
		if(i == max_tokens-1)
		{
			printf("Amount of arguments should be less than %d", max_tokens-1);
			break;
		}
	}
	tokens[i] = NULL;
	return i;
}

char * lookup_command(char * command)
{
	for(int k = 0; command[k] != '\0'; ++k)
		if(command[k] == '/') return command;
	for(int k = 0; environ[k] != NULL; ++k)
	{
		char * name = strtok(environ[k], "=");
		if(!strcmp(name, "PATH"))
		{
			while((name = strtok(NULL, ":")) != NULL)
			{
				//printf("found directory in PATH: '%s'\n", name);
				DIR* dir = opendir(name);
				if(dir == NULL) continue;
				struct dirent* contents;
				while((contents = readdir(dir)) != NULL)
				{
					if((contents->d_type == DT_LNK || contents->d_type == DT_REG) && !strcmp(command, contents->d_name) )
					{
						//printf("Found %s in %s\n", command, name);
						int len = strlen(name) + strlen(command) + 2;
						char * fullname = (char*) malloc(len*sizeof(char));
						fullname[0] = '\0';
						strcat(fullname, name);
						strcat(fullname, "/");
						strcat(fullname, command);
						//printf("Overwrite %s with %s\n", command, fullname);
						return fullname;
					}	
				}
				closedir(dir);
			}
		}
	}
	return command;
}

int read_command(char * line, int bufsize, char ** tokens)
{
	if(fgets(line, bufsize, stdin) == NULL)
		return -1;
	return parse_line(line, tokens, 16);
}

void execute_command(char ** args)
{
	pid_t pid = fork();
	if(pid == -1)
		printf("Failed to fork; errno %d\n", errno);
	else if(pid != 0)
		wait(NULL);
	else
	{
		args[0] = lookup_command(args[0]);
		int result = execve(args[0], args, NULL);
		printf("Failed to find command '%s'.\n", args[0]);
		exit(0);
	}
}


void loop()
{
	char line[1024];
	char* tokens[16];
	do
	{
		fputs("shell> \0", stdout);
		if(!in_loop) break;
		int result = read_command(line, 1024, tokens);
		if(result == -1) break;
		else if(result == 0) continue;
		execute_command(tokens);
	}
	while (1);
}

void sa_handler0(int sig)
{
	in_loop = 0;
}
void signal_handling()
{
	struct sigaction act;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
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
