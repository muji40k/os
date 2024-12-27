#include <unistd.h>
#include <sys/wait.h>

#include "shared_res.h"
#include "unit.h"

#include "cp_sem.h"
#include "buffer.h"
#include "consumer.h"
#include "producer.h"

#define CONSUMERS 3
#define PRODUCERS 3

char proj_id = 1;

int main(int argc, char **argv)
{
    if (argc == 0)
        return EXIT_FAILURE;
    srand(time(NULL));
    pid_t cpid_consumers[CONSUMERS], cpid_producers[PRODUCERS];
    size_t i = 0, j = 0;
    int semfd, shmfd;
    key_t key = ftok(argv[0], proj_id);
    if (init_shared(key, sizeof(buffer_t) + BUF_SIZE, SEM_AMOUNT, &semfd, &shmfd) != EXIT_SUCCESS)
    {
        perror("Unable to init\n");
        return EXIT_FAILURE;
    }
    int rc = EXIT_SUCCESS;
    buffer_t *var = shmat(shmfd, NULL, 0);
    if ((void *)-1 == var)
    {
        perror("Unable to attach shared memory segment (main)\n");
        rc = EXIT_FAILURE;
    }
    if (rc == EXIT_SUCCESS)
    {
        semctl(semfd, EMPTY, SETVAL, BUF_SIZE);
        semctl(semfd, CRITICAL, SETVAL, 1);
        var->current = 'a';
        var->start = (char *)var + sizeof(buffer_t);
        var->end = var->start + BUF_SIZE;
        var->cptr = var->start;
        var->pptr = var->start;
    }
    for (; rc == EXIT_SUCCESS && i < CONSUMERS; i++)
    {
        if ((cpid_consumers[i] = fork()) == 0)
            init_child(semfd, shmfd, consumer, consumer_func);
        else if (cpid_consumers[i] == -1)
        {
            perror("Unable to fork (reader)\n");
            rc = EXIT_FAILURE;
        }
    }
    for (; rc == EXIT_SUCCESS && j < PRODUCERS; j++)
    {
        if ((cpid_producers[j] = fork()) == 0)
            init_child(semfd, shmfd, producer, producer_func);
        else if (cpid_producers[j] == -1)
        {
            perror("Unable to fork (writer)\n");
            rc = EXIT_FAILURE;
        }
    }
    if (rc == EXIT_SUCCESS && (rc = producer(semfd, producer_func, var)) != EXIT_SUCCESS)
        perror("Error during producer process\n");
    if (-1 == shmdt(var))
    {
        perror("Unable to detach shared memory segment (main)\n");
        rc = EXIT_FAILURE;
    }
    pid_t pid;
    int wstatus;
    for (size_t k = 0; j > k; k++)
    {
        pid = waitpid(cpid_producers[k], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    for (size_t k = 0; i > k; k++)
    {
        pid = waitpid(cpid_consumers[k], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    release_shared(&shmfd, &semfd);
    printf("END\n");
    return EXIT_SUCCESS;
}

