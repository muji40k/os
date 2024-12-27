#include "writer.h"

INIT_SEMBUF(wstart_sembuf, {{WRITERS_QUEUE, 1, 0},
                            {READER_ACTIVE, 0, 0},
                            {WRITER_ACTIVE, 0, 0},
                            {WRITER_ACTIVE, 1, 0},
                            {WRITERS_QUEUE, -1, 0}});

int start_write(int semfd)
{
    return semop(semfd, wstart_sembuf, wstart_sembuf_size);
}

INIT_SEMBUF(wstop_sembuf, {{WRITER_ACTIVE, -1, 0}});

int stop_write(int semfd)
{
    return semop(semfd, wstop_sembuf, wstop_sembuf_size);
}

int writer(int semfd, role_func_t func, void *arg)
{
    int rc = EXIT_SUCCESS;
    // printf("(%d): Start writer\n", getpid());
    for (size_t i = 0; rc == EXIT_SUCCESS && i < REPEAT_COUNT_WRITER; i++)
    {
        struct timespec diff = rnd_timespec(MAX_SECONDS_WRITER, MAX_NANOSECONDS_WRITER);
        nanosleep(&diff, NULL);
        // printf("(%d): In writer\n", getpid());
        rc = start_write(semfd);
        if (rc == EXIT_SUCCESS)
        {
            // printf("(%d): Lock writer\n", getpid());
            printf("(%d): WRITE: ", getpid());
            func(arg);
            // printf("(%d): UnLock writer\n", getpid());
            rc = stop_write(semfd);
        }
    }
    return rc;
}

void writer_var_func(void *arg)
{
    (*(int *)arg)++;
    printf("%d\n", *(int *)arg);
}

