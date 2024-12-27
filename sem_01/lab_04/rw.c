#include <unistd.h>
#include <sys/wait.h>

#include "shared_res.h"
#include "unit.h"

#include "rw_sem.h"
#include "writer.h"
#include "reader.h"

#define READERS 3
#define WRITERS 3

char proj_id = 1;

int main(int argc, char **argv)
{
    if (0 == argc)
        return EXIT_FAILURE;
    srand(time(NULL));
    pid_t cpid_readers[READERS], cpid_writers[WRITERS];
    size_t i = 0, j = 0;
    int semfd, shmfd;
    key_t key = ftok(argv[0], proj_id);
    if (init_shared(key, sizeof(int), SEM_AMOUNT, &semfd, &shmfd) != EXIT_SUCCESS)
    {
        perror("Unable to init\n");
        return EXIT_FAILURE;
    }
    int rc = EXIT_SUCCESS;
    int *var = shmat(shmfd, NULL, 0);
    if (var == (void *)-1)
    {
        perror("Unable to attach shared memory segment (main)\n");
        rc = EXIT_FAILURE;
    }
    if (rc == EXIT_SUCCESS)
        *var = 0;
    for (; rc == EXIT_SUCCESS && i < READERS; i++)
    {
        if ((cpid_readers[i] = fork()) == 0)
            init_child(semfd, shmfd, reader, reader_var_func);
        else if (cpid_readers[i] == -1)
        {
            perror("Unable to fork (reader)\n");
            rc = EXIT_FAILURE;
        }
    }
    for (; rc == EXIT_SUCCESS && j < WRITERS; j++)
    {
        if ((cpid_writers[j] = fork()) == 0)
            init_child(semfd, shmfd, writer, writer_var_func);
        else if (cpid_writers[j] == -1)
        {
            perror("Unable to fork (writer)\n");
            rc = EXIT_FAILURE;
        }
    }
    if (rc == EXIT_SUCCESS && (rc = writer(semfd, writer_var_func, var)) != EXIT_SUCCESS)
        perror("Error during write process\n");
    if (-1 == shmdt(var))
    {
        perror("Unable to detach shared memory segment (main)\n");
        rc = EXIT_FAILURE;
    }
    pid_t pid;
    int wstatus;
    for (size_t k = 0; i > k; k++)
    {
        pid = waitpid(cpid_readers[k], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    for (size_t k = 0; j > k; k++)
    {
        pid = waitpid(cpid_writers[k], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    release_shared(&shmfd, &semfd);
    printf("END\n");
    return EXIT_SUCCESS;
}

