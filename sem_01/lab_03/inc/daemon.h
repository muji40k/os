#ifndef _DAEMON_H_
#define _DAEMON_H_

#define _POSIX_C_SOURCE 200112L

#include <syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

void daemonize(const char *cmd);
int  lockfile(int fd);
int  already_running(void);

#endif

