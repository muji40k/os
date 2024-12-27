#include "consumer.h"

INIT_SEMBUF(consumer_start, {{FILLED, -1, 0}, {CRITICAL, -1, 0}});
INIT_SEMBUF(consumer_stop, {{CRITICAL, 1, 0}, {EMPTY, 1, 0}});

int consumer(int semfd, role_func_t func, void *arg)
{
    int rc = EXIT_SUCCESS;
    // printf("(%d): Start consumer\n", getpid());
    for (size_t i = 0; rc == EXIT_SUCCESS && i < REPEAT_COUNT_CONSUMER; i++)
    {
        struct timespec diff = rnd_timespec(MAX_SECONDS_CONSUMER, MAX_NANOSECONDS_CONSUMER);
        nanosleep(&diff, NULL);
        // printf("(%d): In consumer\n", getpid());
        rc = semop(semfd, consumer_start, consumer_start_size);
        if (rc == EXIT_SUCCESS)
        {
            // printf("(%d): Lock consumer\n", getpid());
            printf("(%d): CONSUME: ", getpid());
            func(arg);
            // printf("(%d): UnLock consumer\n", getpid());
            rc = semop(semfd, consumer_stop, consumer_stop_size);
        }
    }
    return rc;
}

void consumer_func(void *arg)
{
    buffer_t *buf = arg;
    if (buf->end == buf->cptr)
        buf->cptr = buf->start;
    printf("%c\n", *buf->cptr);
    buf->cptr++;
}

