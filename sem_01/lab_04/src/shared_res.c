#include "shared_res.h"

static const int perms = S_IRWXU | S_IRWXG | S_IRWXO;

int init_shared(key_t key, size_t shm_size, size_t sem_size, int *const semfd, int *const shmfd)
{
    if (!semfd || !shmfd)
        return EXIT_FAILURE;
    int rc = ((*semfd = semget(key, sem_size, IPC_CREAT | perms)) == -1);
    for (size_t sem = 0; rc == EXIT_SUCCESS && sem_size > sem; sem++)
        rc = semctl(*semfd, sem, SETVAL, 0);
    if (rc == EXIT_SUCCESS)
        rc = ((*shmfd = shmget(key, shm_size, IPC_CREAT | perms)) == -1);
    return rc;
}

int release_shared(int *const shmfd, int *const semfd)
{
    int shmrc = EXIT_SUCCESS, semrc = EXIT_SUCCESS;
    if (shmfd && *shmfd != -1)
    {
        shmrc = (shmctl(*shmfd, IPC_RMID, NULL) == -1);
        *shmfd = -1;
    }
    if (semfd && *semfd != -1)
    {
        semrc = (semctl(*semfd, 0, IPC_RMID) == -1);
        *semfd = -1;
    }
    return semrc || shmrc;
}

