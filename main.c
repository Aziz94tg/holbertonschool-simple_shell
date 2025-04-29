#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


/**
 * is_whitspace - check for spaces
 * @str - strnig to check 
 * Return: 0 if not or 1 if all whitespace
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
	    line = NULL;
            if (isatty(STDIN_FILENO))
                printf("\n");
            break;
        }

        /* Remove newline */
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        if (line[0] == '\0' || is_whitespace(line)) /* Empty input */
            continue;

        child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork");
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

