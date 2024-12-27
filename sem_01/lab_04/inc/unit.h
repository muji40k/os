#ifndef _UNIT_H_
#define _UNIT_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/shm.h>

#include <sys/time.h>
#include <time.h>

typedef void (*role_func_t)(void *arg);
typedef int (*child_func_t)(int semfd, role_func_t func, void *arg);

void show_child_status(const pid_t childpid, const int wstatus);
struct timespec rnd_timespec(int m_sec, int m_nsec);
void init_child(int semfd, int shmfd, child_func_t cfunc, role_func_t func);

#endif

