#define _POSIX_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define N 6
#define BSIZE 100

const char *const (messages[N]) =
{
    "aaa\n", "bbbb\n", "c\n", "dd\n", "1x1\n", "y2yhhhhhhhhh\n"
};

int flag = 0;

void signal_handler(int sig)
{
    if (sig == SIGINT)
        flag = 1;
}

void show_child_status(const pid_t childpid, const int wstatus)
{
    if (childpid == -1)
    {
        printf("Wait error\n");
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
    int fd[2];
    pid_t childpid[N], pid;
    char message[BSIZE];
    if (pipe(fd) != EXIT_SUCCESS)
    {
        perror("Can't pipe\n");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Can't set handler\n");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    for (size_t i = 0; i < N; i++)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        if (childpid[i] == 0)
        {
            if (flag)
            {
                sprintf(message, "%s", messages[i]);
                close(fd[0]);
                write(fd[1], &message, strlen(message));
            }
            exit(EXIT_SUCCESS);
        }
    }
    int wstatus;
    for (size_t i = 0; i < N; i++)
    {
        pid = waitpid(childpid[i], &wstatus, 0);
        show_child_status(pid, wstatus);
    }
    close(fd[1]);
    if (read(fd[0], &message, sizeof(message)) < 0)
    {
        perror("Can't read\n");
        exit(EXIT_FAILURE);
    }
    printf("%s", message);
    return EXIT_SUCCESS;
}

