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

    int fds[MAX_FDS] = {-1};
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.amount > i; i++)
    {
        fds[i] = open("task3.txt", O_CREAT | O_TRUNC | O_WRONLY);
        STAT_WRAP("open", print_path_stat("task3.txt"));

        if (0 > fds[i])
        {
            perror("open error\n");
            rc = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == rc)
        for (char c = 'a'; 'z' >= c; c++)
        {
            write(fds[c % arg.amount], &c, sizeof(char));
            STAT_WRAP("write", print_path_stat("task3.txt"));
        }

    for (size_t i = 0; arg.amount > i; i++)
        if (0 <= fds[i])
        {
            close(fds[i]);
            STAT_WRAP("close", print_path_stat("task3.txt"));
        }

    return EXIT_SUCCESS;
}

