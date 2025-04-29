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
    int found;
    char *path, *path_copy, *dir;
    char full_path[MAX_PATH];

    while (1)
    {
        printf(":) ");
        nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            perror("getline");
            break;
        }

        /* Remove newline */
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        /* Ignore empty input */
        if (line[0] == '\0')
            continue;

        /* If command is an absolute or relative path */
        if (strchr(line, '/'))
        {
            if (access(line, X_OK) == 0)
            {
                pid_t pid;
                pid = fork();
                if (pid == 0)
                {
                    char *args[2];
                    args[0] = line;
                    args[1] = NULL;
                    execve(line, args, environ);
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

        /* Search in PATH */
        path = getenv("PATH");
        if (!path)
        {
            fprintf(stderr, "No PATH found\n");
            continue;
        }

        path_copy = strdup(path);
        if (!path_copy)
        {
            perror("strdup");
            continue;
        }

        dir = strtok(path_copy, ":");
        found = 0;

        while (dir != NULL)
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, line);
            if (access(full_path, X_OK) == 0)
            {
                pid_t pid;
                pid = fork();
                if (pid == 0)
                {
                    char *args[2];
                    args[0] = line;
                    args[1] = NULL;
                    execve(full_path, args, environ);
                    perror("execve");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    wait(NULL);
                }
                found = 1;
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

