#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#include "socket_setup.h"

int main(void)
{
    int sock_serv = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_serv == -1)
    {
        perror("socket error\n");
        return EXIT_FAILURE;
    }
    struct sockaddr serv_addr = setup_server();
    socklen_t len_serv = sizeof(struct sockaddr_un);
    pid_t pid = getpid();
    printf("Init client (%d)\n", pid);
    if (sendto(sock_serv, &pid, sizeof(pid), 0, &serv_addr, len_serv) == -1)
    {
        perror("sendto error\n");
        return EXIT_FAILURE;
    }
    close(sock_serv);
    return EXIT_SUCCESS;
}

