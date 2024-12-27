#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS  12
#define MAX_BUF_SIZE 200

struct data
{
    int valid;
    size_t threads;
    size_t buf_size;
    int fd;
};

struct handle
{
    size_t id;
    const struct data *data;
};

struct data parse_args(int argc, char **argv)
{
    struct data out = {EXIT_SUCCESS, 2, 20, -1};
    char *tmp;

    if (3 < argc)
        out.valid = EXIT_FAILURE;
    else if (1 < argc)
    {
        out.threads = strtoul(argv[1], &tmp, 10);

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
        if (MAX_THREADS < out.threads)
            out.threads = MAX_THREADS;

        if (MAX_BUF_SIZE < out.buf_size)
            out.buf_size = MAX_BUF_SIZE;
    }

    return out;
}

void *thread_func(void *_arg)
{
    struct handle *arg = _arg;
    char buf[MAX_BUF_SIZE], tmp;
    int rc = EXIT_SUCCESS;

    FILE *f = fdopen(arg->data->fd, "r");

    if (!f)
    {
        perror("fdopen error\n");
        rc = EXIT_FAILURE;
    }
    else if (EXIT_SUCCESS != setvbuf(f, buf, _IOFBF, arg->data->buf_size))
    {
        perror("setvbuf error\n");
        rc = EXIT_FAILURE;
    }

    if (EXIT_SUCCESS == rc)
        for (int read = 1; read;)
            if ((read = (1 == fscanf(f, "%c", &tmp))))
                fprintf(stdout, "thread %zu: %c\n", arg->id, tmp);

    if (f)
        fclose(f);

    return NULL;
}

int main(int argc, char **argv)
{
    struct data data = parse_args(argc, argv);

    if (EXIT_SUCCESS != data.valid)
        return EXIT_FAILURE;

    struct handle args[MAX_THREADS];
    pthread_t threads[MAX_THREADS];

    int rc = EXIT_SUCCESS;
    data.fd = open("alphabet", O_RDONLY);

    if (0 > data.fd)
    {
        perror("open error\n");
        rc = EXIT_FAILURE;
    }

    for (size_t i = 0; EXIT_SUCCESS == rc && data.threads > i; i++)
    {
        args[i].id = i + 1;
        args[i].data = &data;

        if (EXIT_SUCCESS != pthread_create(threads + i, NULL, thread_func, args + i))
        {
            perror("pthread_create error\n");
            rc = EXIT_FAILURE;
        }
    }

    for (size_t i = 0; data.threads > i; i++)
        pthread_join(threads[i], NULL);

    if (0 <= data.fd)
        close(data.fd);

    return rc;
}

