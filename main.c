#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

#define DELIM " \t\r\n\a"

char *_getenv(const char *name)
{
	int i = 0, len = strlen(name);

	while (environ[i])
	{
		if (strncmp(environ[i], name, len) == 0 && environ[i][len] == '=')
			return &environ[i][len + 1];
		i++;
	}
	return NULL;
}

char *find_command_path(char *command)
{
	char *path = _getenv("PATH");
	char *token, *full_path;
	size_t cmd_len = strlen(command);

	if (!path)
		return NULL;

	path = strdup(path);
	token = strtok(path, ":");

	while (token)
	{
		full_path = malloc(strlen(token) + cmd_len + 2);
		if (!full_path)
		{
			free(path);
			return NULL;
		}
		strcpy(full_path, token);
		strcat(full_path, "/");
		strcat(full_path, command);

		if (access(full_path, X_OK) == 0)
		{
			free(path);
			return full_path;
		}
		free(full_path);
		token = strtok(NULL, ":");
	}
	free(path);
	return NULL;
}

int main(int argc, char **argv)
{
	char *line = NULL, *command_path = NULL;
	size_t len = 0;
	ssize_t nread;
	int line_number = 0;
	char *args[2];

	(void)argc;

	while (1)
	{
		if (isatty(STDIN_FILENO))
			write(STDOUT_FILENO, ":) ", 3);

		nread = getline(&line, &len, stdin);
		if (nread == -1)
			break;

		line_number++;
		line[nread - 1] = '\0'; /* remove newline */

		if (line[0] == '\0')
			continue;

		if (access(line, X_OK) == 0)
		{
			command_path = strdup(line);
		}
		else
		{
			command_path = find_command_path(line);
		}

		if (!command_path)
		{
			fprintf(stderr, "%s: %d: %s: not found\n", argv[0], line_number, line);
			continue;
		}

		args[0] = command_path;
		args[1] = NULL;

		if (fork() == 0)
		{
			execve(command_path, args, environ);
			perror("execve");
			exit(1);
		}
		else
		{
			wait(NULL);
			free(command_path);
		}
	}
	free(line);
	return 0;
}

