#include "producer.h"

INIT_SEMBUF(producer_start, {{EMPTY, -1, 0}, {CRITICAL, -1, 0}});
INIT_SEMBUF(producer_stop, {{CRITICAL, 1, 0}, {FILLED, 1, 0}});

int producer(int semfd, role_func_t func, void *arg)
{
    int rc = EXIT_SUCCESS;
    // printf("(%d): Start producer\n", getpid());
    for (size_t i = 0; rc == EXIT_SUCCESS && i < REPEAT_COUNT_PRODUCER; i++)
    {
        struct timespec diff = rnd_timespec(MAX_SECONDS_PRODUCER, MAX_NANOSECONDS_PRODUCER);
        nanosleep(&diff, NULL);
        // printf("(%d): In producer\n", getpid());
        rc = semop(semfd, producer_start, producer_start_size);
        if (rc == EXIT_SUCCESS)
        {
            // printf("(%d): Lock producer\n", getpid());
            printf("(%d): PRODUCE: ", getpid());
            func(arg);
            // printf("(%d): UnLock producer\n", getpid());
            rc = semop(semfd, producer_stop, producer_stop_size);
        }
    }
    return rc;
}

void producer_func(void *arg)
{
    buffer_t *buf = arg;
    if ('z' < buf->current)
        buf->current = 'a';
    if (buf->end == buf->pptr)
        buf->pptr = buf->start;
    *buf->pptr = buf->current;
    printf("%c\n", *buf->pptr);
    buf->pptr++;
    buf->current++;
}

