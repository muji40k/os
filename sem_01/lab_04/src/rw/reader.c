#include "reader.h"

INIT_SEMBUF(rstart_sembuf, {{READERS_QUEUE, 1, 0},
                            {WRITER_ACTIVE, 0, 0},
                            {WRITERS_QUEUE, 0, 0},
                            {READER_ACTIVE, 1, 0},
                            {READERS_QUEUE, -1, 0}});

int start_read(int semfd)
{
    return semop(semfd, rstart_sembuf, rstart_sembuf_size);
}

INIT_SEMBUF(rstop_sembuf, {{READER_ACTIVE, -1, 0}});

int stop_read(int semfd)
{
    return semop(semfd, rstop_sembuf, rstop_sembuf_size);
}

int reader(int semfd, role_func_t func, void *arg)
{
    int rc = EXIT_SUCCESS;
    // printf("(%d): Start reader\n", getpid());
    for (size_t i = 0; rc == EXIT_SUCCESS && i < REPEAT_COUNT_READER; i++)
    {
        struct timespec diff = rnd_timespec(MAX_SECONDS_READER, MAX_NANOSECONDS_READER);
        nanosleep(&diff, NULL);
        // printf("(%d): In reader\n", getpid());
        rc = start_read(semfd);
        if (rc == EXIT_SUCCESS)
        {
            // printf("(%d): Lock reader\n", getpid());
            printf("(%d): READ: ", getpid());
            func(arg);
            // printf("(%d): UnLock reader\n", getpid());
            rc = stop_read(semfd);
        }
    }
    return rc;
}

void reader_var_func(void *arg)
{
    printf("%d\n", *(int *)arg);
}

