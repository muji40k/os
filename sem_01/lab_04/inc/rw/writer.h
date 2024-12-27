#ifndef _WRITER_H_
#define _WRITER_H_

#include <unistd.h>

#include "shared_res.h"
#include "unit.h"
#include "rw_sem.h"

#define REPEAT_COUNT_WRITER    4
#define MAX_SECONDS_WRITER     3
#define MAX_NANOSECONDS_WRITER 1000000000

int writer(int semfd, role_func_t func, void *arg);
void writer_var_func(void *arg);

#endif

