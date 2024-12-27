#ifndef _BUFFER_H_
#define _BUFFER_H_

#define BUF_SIZE 100

typedef struct
{
    char current;
    char *cptr;
    char *pptr;
    char *start;
    char *end;
} buffer_t;

#endif

