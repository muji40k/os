#ifndef _CONSUMER_H_
#define _CONSUMER_H_

#include <unistd.h>

#include "shared_res.h"
#include "unit.h"
#include "buffer.h"
#include "cp_sem.h"

#define REPEAT_COUNT_CONSUMER    4
#define MAX_SECONDS_CONSUMER     3
#define MAX_NANOSECONDS_CONSUMER 1000000000

int consumer(int semfd, role_func_t func, void *arg);
void consumer_func(void *arg);

#endif

