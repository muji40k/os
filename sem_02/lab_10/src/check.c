#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int p = 2;

    if (2 < argc)
        return 1;
    else if (1 < argc)
    {
        char *tmp = NULL;
        p = strtoul(argv[1], &tmp, 10);
    }

    int fd = open("alphabet", O_RDONLY);
    char c;
    int k = 0;

    while (read(fd, &c, 1))
    {
        printf("\033[01;%dm%c", 31 + k, c);
        k = (k + 1) % p;
    }

    close(fd);
    printf("\033[00m\n");

    return 0;
}

