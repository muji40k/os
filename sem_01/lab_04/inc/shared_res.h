#ifndef _SHARED_RES_H_
#define _SHARED_RES_H_

#include <stdlib.h>
#include <stdio.h>

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>

#define INIT_SEMBUF(name, ...)                                            \
static struct sembuf name[] = __VA_ARGS__;                                \
static const size_t name ## _size = sizeof(name) / sizeof(struct sembuf)

int init_shared(key_t key, size_t shm_size, size_t sem_size, int *const semfd, int *const shmfd);
int release_shared(int *const shmfd, int *const semfd);

#endif

