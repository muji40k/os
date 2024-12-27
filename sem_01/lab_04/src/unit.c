#include "unit.h"

void show_child_status(const pid_t childpid, const int wstatus)
{
    if (childpid == -1)
    {
        perror("Wait error\n");
        return;
    }
    if (WIFEXITED(wstatus))
        printf("Child (%d) exited with status code %d\n", childpid, WEXITSTATUS(wstatus));
    else if (WIFSTOPPED(wstatus))
        printf("Child (%d) stopped with signal %d\n", childpid, WSTOPSIG(wstatus));
    else if (WIFSIGNALED(wstatus))
        printf("Child (%d) killed with signal %d\n", childpid, WTERMSIG(wstatus));
    else
        printf("Child (%d) exited wrong\n", childpid);
}

struct timespec rnd_timespec(int m_sec, int m_nsec)
{
    struct timespec out;
    out.tv_sec = rand() % m_sec;
    out.tv_nsec = rand() % m_nsec;
    return out;
}

void init_child(int semfd, int shmfd, child_func_t cfunc, role_func_t func)
{
    if (!func || !cfunc)
    {
        perror("Null func\n");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    int rc = EXIT_SUCCESS;
    void *arg = shmat(shmfd, NULL, 0);
    if (arg == (void *)-1)
    {
        perror("Unable to attach to shared memory segment\n");
        rc = EXIT_FAILURE;
    }
    if (rc == EXIT_SUCCESS && (rc = cfunc(semfd, func, arg)) != EXIT_SUCCESS)
        perror("Error during child process\n");
    if (shmdt(arg) == -1)
    {
        perror("Unable to detach shared memory segment\n");
        rc = EXIT_FAILURE;
    }
    exit(rc);
}

