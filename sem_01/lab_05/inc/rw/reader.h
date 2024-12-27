#ifndef _READER_H_
#define _READER_H_

#include <unistd.h>

#include "shared_res.h"
#include "unit.h"
#include "rw_sem.h"

#define REPEAT_COUNT_READER    4
#define MAX_SECONDS_READER     3
#define MAX_NANOSECONDS_READER 1000000000

int reader(int semfd, role_func_t func, void *arg);
void reader_var_func(void *arg);

#endif

