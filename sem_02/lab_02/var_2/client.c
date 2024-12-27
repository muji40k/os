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
    int sock_client = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_client == -1)
    {
        perror("socket error\n");
        return EXIT_FAILURE;
    }
    struct sockaddr serv_addr = setup_server();
    socklen_t len_serv = sizeof(struct sockaddr_un);
    char buffer[100] = {'\0'};
    pid_t pid = getpid();
    sprintf(buffer, "clt%d", pid);
    struct sockaddr client_addr = setup_client(buffer);
    if (bind(sock_client, &client_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind error\n");
        return EXIT_FAILURE;
    }
    printf("[%d] Init client\n", pid);
    if (sendto(sock_client, &pid, sizeof(pid), 0, &serv_addr, len_serv) == -1)
    {
        perror("sendto error\n");
        return EXIT_FAILURE;
    }
    printf("[%d] PID sent\n", pid);
    if (recvfrom(sock_client, buffer, sizeof(buffer), 0, &serv_addr, &len_serv) == -1)
    {
        perror("revfrom error\n");
        return EXIT_FAILURE;
    }
    printf("[%d] Message recerived: %s\n", pid, buffer);
    clean_socket(&client_addr);
    close(sock_client);
    return EXIT_SUCCESS;
}

