#ifndef _STAT_H_
#define _STAT_H_

#ifndef STAT_SILENT
#include <sys/stat.h>
#include <stdio.h>

#define STAT_WRAP(when, statf) printf("[%7s] ", when); (statf)

void print_path_stat(const char *const path)
{
    struct stat buf;
    stat(path, &buf);
    printf("file: %s | inode: %lu, size: %ld\n", path, buf.st_ino, buf.st_size);
}

void print_fd_stat(int fd)
{
    struct stat buf;
    fstat(fd, &buf);
    printf("fd: %d | inode: %lu, size: %ld\n", fd, buf.st_ino, buf.st_size);
}
#else
#define STAT_WRAP(when, statf) {}
#endif

#endif

