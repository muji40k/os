#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#define SERV_PORT 1234

#define MAX_EVENTS 10

int run = 1;

void termination_handler(int signum)
{
    if (SIGINT == signum)
        run = 0;
}

int request_handler(struct epoll_event *ev, int epoll_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_length;
    char buffer[100];
    char msg[100];

    if (ev->events != (EPOLLIN | EPOLLOUT))
        return EXIT_SUCCESS;

    if (recvfrom(ev->data.fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_length) == -1)
    {
        perror("recvfrom error\n");
        return EXIT_FAILURE;
    }

    printf("Received message from client: %s\n", buffer);

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    sprintf(msg, "(%s) -- %s", buffer, asctime(timeinfo));

    if (sendto(ev->data.fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&client_addr, client_length) == -1)
    {
        perror("sendto error\n");
        return EXIT_FAILURE;
    }

    printf("Message sent to client %s: %s\n", buffer, msg);

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ev->data.fd, NULL);
    close(ev->data.fd);

    return EXIT_SUCCESS;
}

int main(void)
{
    struct sigaction new_action;
    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if (sigaction(SIGINT, &new_action, NULL) == -1)
    {
        perror("sigaction error\n");

        return EXIT_FAILURE;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1)
    {
        perror("socket error\n");

        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind error\n");

        return EXIT_FAILURE;
    }

    if (listen(listen_fd, SOMAXCONN) == -1)
    {
        perror("listen error\n");

        return EXIT_FAILURE;
    }

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];

    if (epoll_fd == -1)
    {
        perror("epoll_create1 error\n");

        return EXIT_FAILURE;
    }

    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1)
    {
        perror("epoll_ctl error\n");

        return EXIT_FAILURE;
    }

    for (int conn_fd, event_amount; run;)
    {
        event_amount = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (event_amount == -1 && run)
        {
            perror("epoll_wait error");

            return EXIT_FAILURE;
        }

        for (int i = 0; event_amount > i; i++)
        {
            if (listen_fd == events[i].data.fd)
            {
                conn_fd = accept(listen_fd, NULL, NULL);

                if (conn_fd == -1)
                {
                    perror("accept error");

                    return EXIT_FAILURE;
                }

                printf("New connection accepted\n\n");

                ev.data.fd = conn_fd;
                ev.events = EPOLLIN | EPOLLOUT;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
            }
            else if (request_handler(events + i, epoll_fd) != EXIT_SUCCESS)
                return EXIT_FAILURE;
        }
    }

    close(epoll_fd);
    close(listen_fd);

    return EXIT_SUCCESS;
}

