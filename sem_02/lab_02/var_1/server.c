#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#include <time.h>

#include "socket_setup.h"

#define MAX_CON 5

int main(void)
{
    int sock_serv = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_serv == -1)
    {
        perror("socket error\n");
        return EXIT_FAILURE;
    }
    struct sockaddr serv_addr = setup_server();
    struct sockaddr client_addr;
    socklen_t len_client;
    if (bind(sock_serv, &serv_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind error\n");
        return EXIT_FAILURE;
    }
    pid_t pid;
    for (size_t i = 0; MAX_CON > i; i++)
    {
        if (recvfrom(sock_serv, &pid, sizeof(pid), 0, &client_addr, &len_client) == -1)
        {
            perror("recvfrom error\n");
            return EXIT_FAILURE;
        }
        printf("Request from process: %d\n", pid);
    }
    clean_socket(&serv_addr);
    close(sock_serv);
    return EXIT_SUCCESS;
}

