#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

#define MAX_INPUT 1024
#define MAX_PATH 1024

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1)
    {
        printf(":) ");
        nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            perror("getline");
            break;
        }

        // Remove newline
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        // Ignore empty input
        if (line[0] == '\0')
            continue;

        // If command is an absolute path or contains '/', run it directly
        if (strchr(line, '/'))
        {
            if (access(line, X_OK) == 0)
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    char *argv[] = {line, NULL};
                    execve(line, argv, environ);
                    perror("execve");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    wait(NULL);
                }
            }
            else
            {
                perror(line);
            }
            continue;
        }

        // Search in PATH
        char *path = getenv("PATH");
        if (!path)
        {
            fprintf(stderr, "No PATH found\n");
            continue;
        }

        char *path_copy = strdup(path);
        char *dir = strtok(path_copy, ":");
        int found = 0;
        char full_path[MAX_PATH];

        while (dir)
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, line);
            if (access(full_path, X_OK) == 0)
            {
                found = 1;
                pid_t pid = fork();
                if (pid == 0)
                {
                    char *argv[] = {line, NULL};
                    execve(full_path, argv, environ);
                    perror("execve");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    wait(NULL);
                }
                break;
            }
            dir = strtok(NULL, ":");
        }

        if (!found)
        {
            fprintf(stderr, "%s: command not found\n", line);
        }

        free(path_copy);
    }

    free(line);
    return 0;
}

