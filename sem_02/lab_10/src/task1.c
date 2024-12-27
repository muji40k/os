#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_FDS 10
#define MAX_BUF_SIZE 100

struct arg
{
    int valid;
    size_t amount;
    size_t buf_size;
};

struct arg parse_args(int argc, char **argv)
{
    struct arg out = {EXIT_SUCCESS, 2, 20};
    char *tmp;

    if (3 < argc)
        out.valid = EXIT_FAILURE;
    else if (1 < argc)
    {
        out.amount = strtoul(argv[1], &tmp, 10);

        if ('\0' != *tmp)
            out.valid = EXIT_FAILURE;

        if (EXIT_SUCCESS == out.valid && 2 < argc)
        {
            out.buf_size = strtoul(argv[2], &tmp, 10);

            if ('\0' != *tmp)
                out.valid = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == out.valid)
    {
        if (MAX_FDS < out.amount)
            out.amount = MAX_FDS;

        if (MAX_BUF_SIZE < out.buf_size)
            out.buf_size = MAX_BUF_SIZE;
    }

    return out;
}

int main(int argc, char **argv)
{
    const struct arg arg = parse_args(argc, argv);

    if (EXIT_SUCCESS != arg.valid)
        return EXIT_FAILURE;

    FILE *fds[MAX_FDS] = {NULL};
    char buf[MAX_FDS][MAX_BUF_SIZE], c;
    int fd = open("alphabet", O_RDONLY), rc = EXIT_SUCCESS;

    if (0 > fd)
    {
        perror("open error\n");
        rc = EXIT_FAILURE;
    }

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.amount > i; i++)
    {
        fds[i] = fdopen(fd, "r");

        if (!fds[i])
        {
            perror("fdopen error\n");
            rc = EXIT_FAILURE;
        }
        else if (EXIT_SUCCESS != setvbuf(fds[i], buf[i], _IOFBF, arg.buf_size))
        {
            perror("setvbuf error\n");
            rc = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == rc)
        for (int r = 1; r && !(r = 0);)
            for (size_t i = 0; arg.amount > i; i++)
                if (1 == fscanf(fds[i], "%c", &c))
                {
                    fprintf(stdout, "%c\n", c);
                    r = 1;
                }

    for (size_t i = 0; arg.amount > i; i++)
        if (fds[i])
            fclose(fds[i]);

    if (0 <= fd)
        close(fd);

    return rc;
}

