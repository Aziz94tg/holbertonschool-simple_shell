#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

/* Custom implementation of getenv */
char *_getenv(const char *name)
{
	int i;
	size_t len = strlen(name);

	for (i = 0; environ[i]; i++)
	{
		if (strncmp(environ[i], name, len) == 0 && environ[i][len] == '=')
			return environ[i] + len + 1;
	}
	return NULL;
}

/* Function to check if command exists in PATH */
char *find_command(char *command)
{
	char *path = _getenv("PATH");
	char *path_copy, *dir, *full_path;
	size_t len;

	if (!path)
		return NULL;

	path_copy = strdup(path);
	if (!path_copy)
		return NULL;

	dir = strtok(path_copy, ":");
	while (dir)
	{
		len = strlen(dir) + strlen(command) + 2;
		full_path = malloc(len);
		if (!full_path)
		{
			free(path_copy);
			return NULL;
		}

		sprintf(full_path, "%s/%s", dir, command);
		if (access(full_path, X_OK) == 0)
		{
			free(path_copy);
			return full_path;
		}

		free(full_path);
		dir = strtok(NULL, ":");
	}

	free(path_copy);
	return NULL;
}

int main(void)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	pid_t pid;
	int status;
	char *command_path;

	while (1)
	{
		if (isatty(STDIN_FILENO))
			write(STDOUT_FILENO, ":) ", 3);

		nread = getline(&line, &len, stdin);
		if (nread == -1)
			break;

		if (line[nread - 1] == '\n')
			line[nread - 1] = '\0';

		if (line[0] == '\0')
			continue;

		/* Try to find full command path */
		command_path = NULL;
		if (access(line, X_OK) == 0)
		{
			command_path = strdup(line);
		}
		else
		{
			command_path = find_command(line);
		}

		if (!command_path)
		{
			dprintf(STDERR_FILENO, "%s: No such file or directory\n", line);
			continue;
		}

		pid = fork();
		if (pid == 0)
		{
			char *argv[2];
			argv[0] = command_path;
			argv[1] = NULL;
			execve(command_path, argv, environ);
			perror("execve");
			exit(EXIT_FAILURE);
		}
		else if (pid > 0)
		{
			wait(&status);
			free(command_path);
		}
		else
		{
			perror("fork");
			free(command_path);
		}
	}

	free(line);
	return 0;
}

