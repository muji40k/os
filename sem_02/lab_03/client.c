#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    // serv_addr.sin_addr.s_addr = inet_addr("192.168.15.37");
    serv_addr.sin_port = htons(1234);

    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (serv_fd == -1)
    {
        perror("socket error\n");

        return EXIT_FAILURE;
    }

    if (connect(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect error\n");

        return EXIT_FAILURE;
    }

    char buffer[100];
    sprintf(buffer, "%d", getpid());

    printf("Init client %d\nMessage sent: %s\n", getpid(), buffer);

    if (sendto(serv_fd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("sendto error\n");

        return EXIT_FAILURE;
    }

    if (recvfrom(serv_fd, buffer, sizeof(buffer), 0, NULL, NULL) == -1)
    {
        perror("recvfrom error\n");
        return EXIT_FAILURE;
    }

    printf("Message recerived: %s\n", buffer);

    close(serv_fd);

    return 0;
}

