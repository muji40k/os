#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int fd = open("alphabet", O_CREAT | O_TRUNC | O_WRONLY);

    if (fd <= 0)
        return 1;

    unsigned long num = 1;

    if (1 < argc)
    {
        char *tmp = NULL;
        num = strtoul(argv[1], &tmp, 10);
    }

    for (size_t i = 0; num > i; i++)
        for (char c = 'a'; 'z' >= c; c++)
            write(fd, &c, sizeof(char));

    // write(fd, "\n", sizeof(char));

    close(fd);

    return 0;
}

