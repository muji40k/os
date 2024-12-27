#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// #define STAT_SILENT
#include "stat.h"
#define MAX_FDS 10

struct arg
{
    int valid;
    size_t amount;
};

struct arg parse_args(int argc, char **argv)
{
    struct arg out = {EXIT_SUCCESS, 2};
    char *tmp;

    if (2 < argc)
        out.valid = EXIT_FAILURE;
    else if (1 < argc)
    {
        out.amount = strtoul(argv[1], &tmp, 10);

        if ('\0' != *tmp)
            out.valid = EXIT_FAILURE;
    }

    if (EXIT_SUCCESS == out.valid && MAX_FDS < out.amount)
        out.amount = MAX_FDS;

    return out;
}

int main(int argc, char **argv)
{
    const struct arg arg = parse_args(argc, argv);

    if (EXIT_SUCCESS != arg.valid)
        return EXIT_FAILURE;

    FILE *fds[MAX_FDS] = {NULL};
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.amount > i; i++)
    {
        fds[i] = fopen("task3.txt", "w");
        STAT_WRAP("fopen", print_path_stat("task3.txt"));

        if (!fds[i])
        {
            perror("fopen error\n");
            rc = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == rc)
        for (char c = 'a'; 'z' >= c; c++)
        {
            fprintf(fds[c % arg.amount], "%c", c);
            STAT_WRAP("fprintf", print_path_stat("task3.txt"));
        }

    for (size_t i = 0; arg.amount > i; i++)
        if (fds[i])
        {
            fclose(fds[i]);
            STAT_WRAP("fclose", print_path_stat("task3.txt"));
        }

    return rc;
}

