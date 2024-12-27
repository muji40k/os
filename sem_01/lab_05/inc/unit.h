#ifndef _UNIT_H_
#define _UNIT_H_

#include <processthreadsapi.h>
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>

typedef void (*role_func_t)(void *arg);
typedef int (*child_func_t)(int semfd, role_func_t func, void *arg);

void show_child_status(const pid_t childpid, const int wstatus);
int rnd_timespec(int m_sec, int m_nsec);
void init_child(child_func_t cfunc, role_func_t func, void *arg);

#endif

