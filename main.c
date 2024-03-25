#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/syscall.h>

extern char** environ;

const int line_size = 1024;
const int max_tokens = 15;

int in_loop = 1;

char* builtin_cmds[] = {
	"cd",
	"help",
	"exit"
};

// returns amount of tokens parsed
int parse_line(char* line, char** tokens, int max_tokens)
{
	int i = 0;
	const char* delim = " \t\n";
	tokens[i] = strtok(line, delim);
	while(tokens[i] != NULL && i < max_tokens)
	{
		tokens[++i] = strtok(NULL, delim);
		if(i == max_tokens)
		{
			printf("Amount of arguments should be less than %d", max_tokens);
			break;
		}
	}
	tokens[i] = NULL;
	return i;
}

char* lookup_cmd(char* cmd)
{
	for(int k = 0; cmd[k] != '\0'; ++k)
		if(cmd[k] == '/') return cmd;
	for(int k = 0; environ[k] != NULL; ++k)
	{
		char* name = strtok(environ[k], "=");
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
					if((contents->d_type == DT_LNK || contents->d_type == DT_REG) && !strcmp(cmd, contents->d_name) )
					{
						//printf("Found %s in %s\n", cmd, name);
						int len = strlen(name) + strlen(cmd) + 2;
						char* fullname = (char*) malloc(len*sizeof(char));
						fullname[0] = '\0';
						strcat(fullname, name);
						strcat(fullname, "/");
						strcat(fullname, cmd);
						//printf("Overwrite %s with %s\n", cmd, fullname);
						return fullname;
					}	
				}
				closedir(dir);
			}
		}
	}
	return cmd;
}

int read_cmd(char* line, int bufsize, char** tokens)
{
	if(fgets(line, bufsize, stdin) == NULL)
		return -1;
	return parse_line(line, tokens, max_tokens);
}

int builtin_cmd(char* cmd)
{
	int k;
	for(k = 0; builtin_cmds[k] != NULL; ++k)
		if(!strcmp(cmd, builtin_cmds[k]))
			return k;
	return -1;
}

void print_help()
{
	printf("Available builtin commands:");
	for(int k = 0; builtin_cmds[k] != NULL; ++k)
		printf(" %s", builtin_cmds[k]);
	printf("\n");
	printf("Shell supports small subset of bash commands with multiple arguments.\n");
}

void execute_builtin(char** args)
{
	if(!strcmp(args[0], "cd"))
		chdir(args[1]);
	else if(!strcmp(args[0], "help"))
		print_help();
	else if(!strcmp(args[0], "exit"))
		kill(0, SIGINT);
}

void execute_cmd(char** args)
{
	int k = -1;
	if((k = builtin_cmd(args[0])) != -1)
		execute_builtin(args);
	else
	{
		pid_t pid = fork();
		if(pid == -1)
			printf("Failed to fork; errno %d.\n", errno);
		else if(pid != 0)
			wait(NULL);
		else
		{
			args[0] = lookup_cmd(args[0]);
			int result = execve(args[0], args, NULL);
			printf("Failed to find command '%s'.\n", args[0]);
			exit(EXIT_FAILURE);
		}
	}
}


void loop()
{
	char line[line_size];
	char* tokens[max_tokens+1];
	do
	{
		printf("shell> ");
		if(!in_loop) break;
		int result = read_cmd(line, line_size, tokens);
		if(result == -1) break;
		else if(result == 0) continue;
		execute_cmd(tokens);
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

int main(int argc, char* argv[])
{
	//signal_handling();
	printf("Start shell.\n");
	loop();
	printf("Finish shell.\n");
	return 0;
}
