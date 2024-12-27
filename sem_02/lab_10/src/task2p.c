#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS  12

struct arg
{
    int valid;
    size_t threads;
};

struct arg parse_args(int argc, char **argv)
{
    struct arg out = {EXIT_SUCCESS, 2};
    char *tmp;

    if (2 < argc)
        out.valid = EXIT_FAILURE;
    else if (1 < argc)
    {
        out.threads = strtoul(argv[1], &tmp, 10);

        if ('\0' != *tmp)
            out.valid = EXIT_FAILURE;
    }

    if (EXIT_SUCCESS == out.valid && MAX_THREADS < out.threads)
        out.threads = MAX_THREADS;

    return out;
}

void *thread_func(void *_arg)
{
    int fd = open("alphabet", O_RDONLY);
    char tmp, buf[30];
    ssize_t len;

    if (0 > fd)
    {
        perror("open error\n");

        return NULL;
    }

    for (int read1 = 1; read1;)
        if ((read1 = (1 == read(fd, &tmp, 1))))
        {
            len = sprintf(buf, "thread %zu: %c\n", *(size_t *)_arg, tmp);
            write(1, buf, len);
        }

    close(fd);

    return NULL;
}

int main(int argc, char **argv)
{
    const struct arg arg = parse_args(argc, argv);

    if (EXIT_SUCCESS != arg.valid)
        return EXIT_FAILURE;

    size_t args[MAX_THREADS];
    pthread_t threads[MAX_THREADS];
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.threads > i; i++)
    {
        args[i] = i + 1;

        if (EXIT_SUCCESS != pthread_create(threads + i, NULL, thread_func, args + i))
        {
            perror("pthread_create error\n");
            rc = EXIT_FAILURE;
        }
    }

    for (size_t i = 0; arg.threads > i; i++)
        pthread_join(threads[i], NULL);

    return rc;
}

