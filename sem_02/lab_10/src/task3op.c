#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

// #define STAT_SILENT
#include "stat.h"

#define MAX_THREADS 10

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

struct data
{
    char c;
    pthread_mutex_t mutex;
    const struct arg *arg;
};

struct handle
{
    size_t id;
    struct data *data;
};

void *thread_func(void *_arg)
{
    struct handle *arg = _arg;
    char tmp[30];
    ssize_t len;

    int fd = open("task3.txt", O_WRONLY);
    STAT_WRAP("open", print_path_stat("task3.txt"));

    if (0 > fd)
    {
        perror("open error\n");

        return NULL;
    }

    for (int run = 1, rc = EXIT_SUCCESS; EXIT_SUCCESS == rc && run;)
    {
        rc = pthread_mutex_lock(&arg->data->mutex);

        if (EXIT_SUCCESS == rc)
        {
            if ('z' < arg->data->c)
                run = 0;
            else if ((arg->data->c - 'a') % arg->data->arg->threads == arg->id - 1)
            {
                len = sprintf(tmp, "thread %zu: %c\n", arg->id, arg->data->c++);
                write(fd, tmp, len);
                STAT_WRAP("write", print_path_stat("task3.txt"));
            }

            if (EXIT_SUCCESS != pthread_mutex_unlock(&arg->data->mutex))
            {
                perror("mutex_unlock error\n");
                rc = EXIT_FAILURE;
            }
        }
        else
            perror("mutex_lock error\n");
    }

    close(fd);
    STAT_WRAP("close", print_path_stat("task3.txt"));

    return NULL;
}

int main(int argc, char **argv)
{
    const struct arg arg = parse_args(argc, argv);

    if (EXIT_SUCCESS != arg.valid)
        return EXIT_FAILURE;

    if (EXIT_SUCCESS != close(open("task3.txt", O_CREAT | O_TRUNC)))
        return EXIT_FAILURE;

    struct data data = {
        .c = 'a',
        .mutex = {{0}},
        .arg = &arg
    };

    if (EXIT_SUCCESS != pthread_mutex_init(&data.mutex, NULL))
    {
        perror("mutex_init error\n");
        return EXIT_FAILURE;
    }

    struct handle args[MAX_THREADS];
    pthread_t threads[MAX_THREADS];
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; EXIT_SUCCESS == rc && arg.threads > i; i++)
    {
        args[i].id = i + 1;
        args[i].data = &data;

        if (EXIT_SUCCESS != pthread_create(threads + i, NULL, thread_func, args + i))
        {
            perror("pthread_create error\n");
            rc = EXIT_FAILURE;
        }
    }

    for (size_t i = 0; arg.threads > i; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&data.mutex);

    return EXIT_SUCCESS;
}

