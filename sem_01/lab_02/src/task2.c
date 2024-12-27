#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 4

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

int main(void)
{
    pid_t childpid[N], pid;
    for (size_t i = 0; i < N; i++)
    {
        childpid[i] = fork();
        if(childpid[i] == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        if (childpid[i] == 0)
        {
            printf("pid - %d grppid - %d ppid - %d\n", getpid(), getpgrp(), getppid());
            exit(EXIT_SUCCESS);
        }
        else
            printf("pid - %d grppid - %d cpid - %d\n", getpid(), getpgrp(), childpid[i]);
    }
    int wstatus;
    for (size_t i = 0; i < N; i++)
    {
        pid = waitpid(childpid[i], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    return EXIT_SUCCESS;
}
