#include <windows.h>
#include <stdio.h>
#include <time.h>

#define READERS 3
#define WRITERS 3

#define READERS_REPEAT 4
#define WRITERS_REPEAT 4

#define READERS_TIME 4, 1000
#define WRITERS_TIME 4, 1000

typedef void (*role_func_t)(void *);

typedef struct
{
    HANDLE can_read;
    HANDLE can_write;
    LONG readers_active;
    LONG is_write;
    LONG readers_queue;
    LONG writers_queue;
    HANDLE mutex;
} rw_arg_t;

typedef DWORD (*unit_func_t)(rw_arg_t *, role_func_t, void *);

int rnd_time(int m_sec, int m_msec)
{
    return rand() % m_sec * 1e3 + rand() % m_msec;;
}

void start_read(rw_arg_t *arg)
{
    InterlockedIncrement(&arg->readers_queue);
    if (arg->is_write || arg->writers_queue)
        ResetEvent(arg->can_read);
    WaitForSingleObject(arg->can_read, INFINITE);
    InterlockedIncrement(&arg->readers_active);
    SetEvent(arg->can_read);
    InterlockedDecrement(&arg->readers_queue);
}

void stop_read(rw_arg_t *arg)
{
    InterlockedDecrement(&arg->readers_active);
    if (0 == arg->readers_active)
        SetEvent(arg->can_write);
}

DWORD reader_func(rw_arg_t *rw_arg, role_func_t func, void *arg)
{
    srand(time(NULL) + GetCurrentThreadId());
    // printf("(%ld): Reader start\n", GetCurrentThreadId());
    for (size_t i = 0; i < READERS_REPEAT; i++)
    {
        Sleep(rnd_time(READERS_TIME));
        // printf("(%ld): Reader in\n", GetCurrentThreadId());
        start_read(rw_arg);
        // printf("(%ld): Reader Lock\n", GetCurrentThreadId());
        printf("(%ld): READ: ", GetCurrentThreadId());
        func(arg);
        // printf("(%ld): Reader UnLock\n", GetCurrentThreadId());
        stop_read(rw_arg);
    }
    return EXIT_SUCCESS;
}

void reader_var_func(void *arg)
{
    printf("%d\n", *(int *)arg);
}

void start_write(rw_arg_t *arg)
{
    InterlockedIncrement(&arg->writers_queue);
    if (arg->readers_active || arg->is_write)
        ResetEvent(arg->can_write);
    WaitForSingleObject(arg->can_write, INFINITE);
    WaitForSingleObject(arg->mutex, INFINITE);
    InterlockedIncrement(&arg->is_write);
    InterlockedDecrement(&arg->writers_queue);
}

void stop_write(rw_arg_t *arg)
{
    InterlockedDecrement(&arg->is_write);
    ReleaseMutex(arg->mutex);
    if (arg->readers_queue)
        SetEvent(arg->can_read);
    else
        SetEvent(arg->can_write);
}

DWORD writer_func(rw_arg_t *rw_arg, role_func_t func, void *arg)
{
    srand(time(NULL) + GetCurrentThreadId());
    // printf("(%ld): Writer start\n", GetCurrentThreadId());
    for (size_t i = 0; i < WRITERS_REPEAT; i++)
    {
        Sleep(rnd_time(WRITERS_TIME));
        // printf("(%ld): Writer in\n", GetCurrentThreadId());
        start_write(rw_arg);
        // printf("(%ld): Writer Lock\n", GetCurrentThreadId());
        printf("(%ld): WRITE: ", GetCurrentThreadId());
        func(arg);
        // printf("(%ld): Writer UnLock\n", GetCurrentThreadId());
        stop_write(rw_arg);
    }
    return EXIT_SUCCESS;
}

void writer_var_func(void *arg)
{
    (*(int *)arg)++;
    printf("%d\n", *(int *)arg);
}

typedef struct
{
    rw_arg_t *rw;
    unit_func_t unit;
    role_func_t func;
    void *arg;
} thread_arg_t;

DWORD thread_func(LPVOID lpParam)
{
    thread_arg_t *arg = lpParam;
    return arg->unit(arg->rw, arg->func, arg->arg);
}

int main(void)
{
    HANDLE threads_readers[READERS];
    HANDLE threads_writers[WRITERS];
    rw_arg_t rw_arg;
    int var = 0;
    rw_arg.can_read = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!rw_arg.can_read)
    {
        printf("Create Event error: %ld\n", GetLastError());
        return EXIT_FAILURE;
    }
    rw_arg.can_write = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (!rw_arg.can_write)
    {
        printf("Create Event error: %ld\n", GetLastError());
        return EXIT_FAILURE;
    }
    rw_arg.mutex = CreateMutex(NULL, FALSE, NULL);
    if (!rw_arg.mutex)
    {
        printf("Create Mutex error: %ld\n", GetLastError());
        return EXIT_FAILURE;
    }
    rw_arg.readers_active = 0;
    rw_arg.is_write = FALSE;
    rw_arg.readers_queue = 0;
    rw_arg.writers_queue = 0;
    thread_arg_t r_arg = {&rw_arg, reader_func, reader_var_func, &var};
    thread_arg_t w_arg = {&rw_arg, writer_func, writer_var_func, &var};
    for (size_t i = 0; i < READERS; i++)
    {
        threads_readers[i] = CreateThread(NULL, 0, thread_func, &r_arg, 0, NULL);

        if (!threads_readers[i])
        {
            printf("Create Thread error: %ld\n", GetLastError());
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0; i < WRITERS; i++)
    {
        threads_writers[i] = CreateThread(NULL, 0, thread_func, &w_arg, 0, NULL);
        if (!threads_writers[i])
        {
            printf("Create Thread error: %ld\n", GetLastError());
            return EXIT_FAILURE;
        }
    }
    writer_func(&rw_arg, writer_var_func, &var);
    for (size_t i = 0; i < READERS; i++)
    {
        WaitForSingleObject(threads_readers[i], INFINITE);
        CloseHandle(threads_readers[i]);
    }
    for (size_t i = 0; i < WRITERS; i++)
    {
        WaitForSingleObject(threads_writers[i], INFINITE);
        CloseHandle(threads_writers[i]);
    }
    CloseHandle(rw_arg.can_read);
    CloseHandle(rw_arg.can_write);
    CloseHandle(rw_arg.mutex);
    printf("END\n");
    return EXIT_SUCCESS;
}

