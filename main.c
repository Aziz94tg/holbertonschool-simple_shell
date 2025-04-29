#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

/**
 * main - Simple shell 0.1
 *
 * Return: Always 0
 */
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    pid_t child_pid;

    while (1)
    {
        if (isatty(STDIN_FILENO)) /* Only print prompt in interactive mode */
            printf("#cisfun$ ");

        nread = getline(&line, &len, stdin);
        if (nread == -1) /* EOF (Ctrl+D) or error */
        {
            free(line);
            if (isatty(STDIN_FILENO))
                printf("\n");
            break;
        }

        /* Remove newline */
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        if (line[0] == '\0') /* Empty input */
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
            char *argv[2];

	    argv[0] = line;
	    argv[1] = NULL;

            if (execve(line, argv, environ) == -1)
            {
                perror("./shell"); /* as required in error format */
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            wait(NULL); /* Parent waits for child */
        }
    }

    free(line);
    return (0);
}

