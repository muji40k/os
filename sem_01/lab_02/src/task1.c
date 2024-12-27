#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 4

int main(void)
{
    pid_t childpid[N];
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
            sleep(1);
            printf("pid - %d grppid - %d ppid - %d\n", getpid(), getpgrp(), getppid());
            exit(EXIT_SUCCESS);
        }
        else
            printf("pid - %d grppid - %d cpid - %d\n", getpid(), getpgrp(), childpid[i]);
    }
    return EXIT_SUCCESS;
}

