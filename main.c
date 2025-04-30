#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

extern char **environ;

#define MAX_TOKENS 64
#define DELIM " \t\r\n\a"

char **parse_line(char *line)
{
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    char *token;
    int position = 0;

    if (!tokens)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL && position < MAX_TOKENS - 1)
    {
        tokens[position++] = token;
        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *find_path_in_environ(void)
{
    int i = 0;
    size_t len = strlen("PATH=");

    while (environ[i])
    {
        if (strncmp(environ[i], "PATH=", len) == 0)
            return environ[i] + len;
        i++;
    }
    return NULL;
}

char *find_path(char *command)
{
    char *path = find_path_in_environ();
    char *path_copy, *dir, full_path[1024];
    struct stat st;

    if (!path || *path == '\0')  /* Handle empty PATH */
        return NULL;

    path_copy = strdup(path);
    if (!path_copy)
        return NULL;

    dir = strtok(path_copy, ":");
    while (dir != NULL)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        if (stat(full_path, &st) == 0 && (st.st_mode & S_IXUSR))
        {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

int main(int argc, char **argv)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_number = 0;
    char **args;
    char *command_path;
    pid_t child_pid;
    int status = 0;

    (void)argc;

    while (1)
    {
        if (isatty(STDIN_FILENO))
            write(STDOUT_FILENO, ":) ", 3);

        read = getline(&line, &len, stdin);
        if (read == -1)
            break;

        line_number++;
        if (line[read - 1] == '\n')
            line[read - 1] = '\0';

        if (line[0] == '\0')
            continue;

        args = parse_line(line);

        if (args[0] == NULL)
        {
            free(args);
            continue;
        }

	if (strcmp(args[0], "exit") == 0)
	{
		free(args);
		free(line);
		exit(WEXITSTATUS(status));
	}

        command_path = NULL;

        if (strchr(args[0], '/') != NULL && access(args[0], X_OK) == 0)
        {
            command_path = strdup(args[0]);
        }
        else
        {
            command_path = find_path(args[0]);
            if (!command_path)
            {
                fprintf(stderr, "%s: %d: %s: not found\n", argv[0], line_number, args[0]);
                free(args);
		free(line);
		exit(127);
                continue;
            }
        }

        child_pid = fork();
        if (child_pid == 0)
        {
            execve(command_path, args, environ);
            perror("execve");
            exit(127);
        }
        else
        {
            wait(&status);
        }

        free(args);
        free(command_path);
    }

    free(line);
    return 0;
}

