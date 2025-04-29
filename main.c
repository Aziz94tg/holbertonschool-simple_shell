#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

int main(int argc, char **argv)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_number = 0;
    char **args;

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

        if (access(args[0], X_OK) != 0)
        {
            fprintf(stderr, "%s: %d: %s: not found\n", argv[0], line_number, args[0]);
            free(args);
            continue;
        }

        if (fork() == 0)
        {
            execve(args[0], args, environ);
            perror("execve");
            exit(127);
        }
        else
        {
            wait(NULL);
            free(args);
        }
    }

    free(line);
    return 0;
}

