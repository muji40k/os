#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BSIZE     100
#define CHILD_NUM   3

const char *const child_msg[CHILD_NUM] = {"a", "aaaaaaaa", "aaaaaaaaaaaaaa"};
const char *const parent_msg[CHILD_NUM] = {"bbbbb", "b", "bbbbbbbbbbbbbbb"};

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
    int sockets[2];
    char buffer[BSIZE] = {0};
    int pid[CHILD_NUM];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
    {
        perror("socketpair error\n");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; CHILD_NUM > i; i++)
    {
        pid[i] = fork();
        if (pid[i] == -1)
        {
            perror("fork error\n");
            return EXIT_FAILURE;
        }
        if (pid[i] == 0)
        {
            close(sockets[0]);
            sprintf(buffer, "%s", child_msg[i]);
            if (write(sockets[1], buffer, strlen(buffer) + 1) == -1)
            {
                perror("write error\n");
                exit(EXIT_FAILURE);
            }
            printf("Child  (%d) sent message: %s\n", getpid(), buffer);
            ssize_t k = read(sockets[1], buffer, sizeof(buffer));
            if (k == -1)
            {
                perror("read error\n");
                exit(EXIT_FAILURE);
            }
            printf("Child  (%d) received: %s\n", getpid(), buffer);
            close(sockets[1]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            ssize_t k = read(sockets[0], buffer, sizeof(buffer));
            if (k == -1)
            {
                perror("write error\n");
                exit(EXIT_FAILURE);
            }
            printf("Parent (%d) received: %s\n", getpid(), buffer);
            sprintf(buffer, "%s", parent_msg[i]);
            if (write(sockets[0], buffer, strlen(buffer) + 1) == -1)
            {
                perror("write error\n");
                exit(EXIT_FAILURE);
            }
            printf("Parent (%d) sent message: %s\n", getpid(), buffer);
        }
    }
    close(sockets[1]);
    close(sockets[0]);
    int wstatus;
    pid_t tmp_pid;
    for (size_t i = 0; CHILD_NUM > i; i++)
    {
        tmp_pid = waitpid(pid[i], &wstatus, 0);
        show_child_status(tmp_pid, wstatus);
    }
    return EXIT_SUCCESS;
}


