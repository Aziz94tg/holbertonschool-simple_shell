#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 64

extern char **environ;

/**
 * is_whitespace - checks if a string is all whitespace
 * @str: input string
 * Return: 1 if all whitespace, 0 otherwise
 */
int is_whitespace(const char *str)
{
	while (*str)
	{
		if (*str != ' ' && *str != '\t' && *str != '\n' &&
		    *str != '\r' && *str != '\v' && *str != '\f')
			return (0);
		str++;
	}
	return (1);
}

/**
 * main - Simple shell 0.1
 * Return: Always 0
 */
int main(void)
{
	char *line = NULL, *token = NULL;
	size_t len = 0;
	ssize_t nread;
	pid_t child_pid;
	char *cmd_argv[MAX_ARGS];
	int i;

	while (1)
	{
		if (isatty(STDIN_FILENO))
			printf("#cisfun$ ");

		nread = getline(&line, &len, stdin);
		if (nread == -1)
		{
			if (isatty(STDIN_FILENO))
				printf("\n");
			break;
		}

		if (line[nread - 1] == '\n')
			line[nread - 1] = '\0';

		if (line[0] == '\0' || is_whitespace(line))
			continue;

		child_pid = fork();
		if (child_pid == -1)
		{
			perror("fork");
			free(line);
			exit(EXIT_FAILURE);
		}

		if (child_pid == 0)
		{
			i = 0;
			token = strtok(line, " \t\r\n");
			while (token && i < MAX_ARGS - 1)
			{
				cmd_argv[i++] = token;
				token = strtok(NULL, " \t\r\n");
			}
			cmd_argv[i] = NULL;

			if (cmd_argv[0] == NULL)
				exit(EXIT_SUCCESS);

			if (execve(cmd_argv[0], cmd_argv, environ) == -1)
			{
				perror(cmd_argv[0]);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			wait(NULL);
		}
	}

	free(line);
	return (0);
}

