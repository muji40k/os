#ifndef _PRODUCER_H_
#define _PRODUCER_H_

#include <unistd.h>

#include "shared_res.h"
#include "unit.h"
#include "buffer.h"
#include "cp_sem.h"

#define REPEAT_COUNT_PRODUCER    4
#define MAX_SECONDS_PRODUCER     3
#define MAX_NANOSECONDS_PRODUCER 1000000000

int producer(int semfd, role_func_t func, void *arg);
void producer_func(void *arg);

#endif

