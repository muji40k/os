#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

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
    char buf[2] = "\0\n";
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.amount > i; i++)
    {
        fds[i] = open("alphabet", O_RDONLY);

        if (0 > fds[i])
        {
            perror("open error");
            rc = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == rc)
        for (int r = 1; r && !(r = 0);)
            for (size_t i = 0; arg.amount > i; i++)
                if (1 == read(fds[i], buf, 1))
                {
                    write(1, buf, 2);
                    r = 1;
                }

    for (size_t i = 0; arg.amount > i; i++)
        if (fds[i] >= 0)
            close(fds[i]);

    return rc;
}

