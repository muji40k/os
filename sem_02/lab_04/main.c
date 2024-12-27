#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <inttypes.h>
#include <dirent.h>

#include <locale.h>
#include "cli.h"

#define BUF_SIZE 100

static int run = 0;

void termination_handler(int signum)
{
    if (SIGINT == signum)
        run = 0;
}

static char file_buf[BUF_SIZE];
static char buffer[BUF_SIZE];

void prompt(void *arg)
{
    wprintf(L"%d $ ", *(pid_t *)arg);
}

int set_pid(const size_t n, wchar_t **args, void *arg)
{
    if (1 != n)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    wchar_t *tmp;
    unsigned long val = wcstoul(args[0], &tmp, 10);

    if ('\0' != *tmp)
    {
        wprintf(L"Pid is a nubmer\n");

        return EXIT_SUCCESS;
    }

    *(pid_t *)arg = val;

    return EXIT_SUCCESS;
}

int show_cwd(const size_t n, wchar_t **args, void *arg)
{
    sprintf(file_buf, "/proc/%d/cwd", *(pid_t *)arg);

    ssize_t rclen = readlink(file_buf, buffer, BUF_SIZE - 1);

    if (1 != rclen)
        buffer[rclen] = '\0';

    wprintf(L"%s\n", buffer);

    return EXIT_SUCCESS;
}

int show_env(const size_t n, wchar_t **args, void *arg)
{
    sprintf(file_buf, "/proc/%d/environ", *(pid_t *)arg);
    FILE *environ_file = fopen(file_buf, "r");
    ssize_t len;

    while (0 < (len = fread(buffer, 1, BUF_SIZE - 1, environ_file)))
    {
        for (size_t i = 0; len > i; i++)
            if (0 == buffer[i])
                buffer[i] = '\n';

        buffer[len] = '\0';
        wprintf(L"%s", buffer);
    }

    if (feof(environ_file))
    {
        for (size_t i = 0; len > i; i++)
            if (0 == buffer[i])
                buffer[i] = '\n';

        buffer[len] = '\0';
        wprintf(L"%s", buffer);
    }

    fclose(environ_file);

    return EXIT_SUCCESS;
}

int plain_read_line(FILE *file, char **const line, size_t *const size)
{
    if (!line || !size)
        return EXIT_FAILURE;

    if (*line)
        return EXIT_FAILURE;

    size_t allocated_size = BUF_SIZE, cur_size = 0;
    char *tmp = NULL;
    int rc = EXIT_SUCCESS;
    *line = calloc(BUF_SIZE, sizeof(char));

    if (!line)
        rc = EXIT_FAILURE;

    for (int read = 1; EXIT_SUCCESS == rc && read;)
    {
        if (NULL == fgets(*line + cur_size, allocated_size - cur_size, file))
            rc = ERROR_INPUT_CLI;

        if (EXIT_SUCCESS == rc)
        {
            cur_size = strlen(*line);

            if ('\n' == (*line)[cur_size - 1])
            {
                (*line)[--cur_size] = '\0';
                read = 0;
            }
            else
            {
                allocated_size *= 2;
                tmp = realloc(*line, allocated_size * sizeof(wchar_t));

                if (!tmp)
                    rc = EXIT_FAILURE;
                else
                    *line = tmp;
            }
        }
        else
        {
            if (feof(file))
                rc = EXIT_FAILURE;
        }
    }

    if (EXIT_SUCCESS == rc && allocated_size > cur_size + 1)
    {
        allocated_size = cur_size + 1;
        tmp = realloc(*line, allocated_size * sizeof(char));

        if (!tmp)
            rc = EXIT_FAILURE;
        else
            *line = tmp;
    }

    if (EXIT_SUCCESS != rc)
    {
        free(*line);
        allocated_size = 0;
        *line = NULL;
    }

    *size = allocated_size;

    return rc;
}

int show_maps(const size_t n, wchar_t **args, void *arg)
{
    sprintf(file_buf, "/proc/%d/maps", *(pid_t *)arg);
    FILE *maps_file = fopen(file_buf, "r");
    FILE *file = stdout;

    unsigned long long vsize = 0, start, end;
    char *line = NULL;
    char *tmp = NULL, *item = NULL;
    size_t len;

    if (1 == n)
    {
        wcstombs(buffer, args[0], BUF_SIZE - 1);
        file = fopen(buffer, "w");

        if (!file)
        {
            wprintf(L"No such file %s", args[0]);

            return EXIT_SUCCESS;
        }
    }

    for (int rc = EXIT_SUCCESS; EXIT_SUCCESS == rc && !feof(maps_file);)
    {
        rc = plain_read_line(maps_file, &line, &len);

        if (EXIT_SUCCESS == rc)
        {
            fwprintf(file, L"%s\n", line);
            tmp = line;
            tmp = strtok(tmp, " ");
            item = strtok(tmp, "-");
            start = strtoul(item, &tmp, 16);
            item = strtok(NULL, "-");
            end = strtoul(item, &tmp, 16);

            vsize += end - start;

            free(line);
            line = NULL;
        }
    }

    wprintf(L"Total: %llu\n", vsize);
    wprintf(L"Pages: %llu\n", vsize / getpagesize());
    fclose(maps_file);

    if (1 == n)
        fclose(file);

    return EXIT_SUCCESS;
}

#define WRONG_ARG 100

struct file_range
{
    int valid;
    long start;
    long end;
};

struct file_range parse_range(wchar_t *range)
{
    struct file_range out = {EXIT_SUCCESS, 0, 0};

    wchar_t *copy = malloc(wcslen(range) * sizeof(wchar_t));
    wchar_t *base = copy;

    if (!copy)
    {
        wprintf(L"Malloc error\n");
        out.valid = EXIT_FAILURE;
        return out;
    }

    wcscpy(copy, range);

    wchar_t *sarg[2] = {NULL};
    wchar_t *state = NULL;
    wchar_t *tmp = wcstok(copy, L"-", &state);

    for (size_t i = 0; 2 > i; i++)
    {
        sarg[i] = tmp;
        copy = state;
        tmp = wcstok(copy, L"-", &state);
    }

    if (NULL != tmp)
    {
        wprintf(L"Wrong arg\n");
        out.valid = WRONG_ARG;
        return out;
    }

    unsigned long start = wcstoul(sarg[0], &tmp, 16);

    if ('\0' != *tmp)
    {
        wprintf(L"Virtual address is hex\n");
        out.valid = WRONG_ARG;
        return out;
    }

    unsigned long end = wcstoul(sarg[1], &tmp, 16);

    if ('\0' != *tmp)
    {
        wprintf(L"Virtual address is hex\n");
        out.valid = WRONG_ARG;
        return out;
    }

    free(base);

    out.start = start / getpagesize() * sizeof(uint64_t);
    out.end = end / getpagesize() * sizeof(uint64_t);

    if (start > end)
    {
        start = out.start;
        out.start = out.end;
        out.end = start;
    }

    return out;
}

void print_bits(uint64_t num, FILE *file)
{
    for (size_t i = 0; 8 * sizeof(uint64_t) > i; i++)
    {
        int bit = num & 1;
        fwprintf(file, L"%d", bit);
        num >>= 1;
    }

    wprintf(L"\n");
}

struct pagemap
{
    int active;
    int swap;
    int fm_sh;
    int wprotected;
    int emapped;
    int dirty;
    union
    {
        uint64_t frame;

        struct
        {
            int type;
            uint64_t offset;
        } swap_info;
    } rest;
};

struct pagemap get_pagemap(uint64_t num)
{
    struct pagemap out = {0, 0, 0, 0, 0, 0, {0}};

    out.active = 0 != (num & UINT64_C(9223372036854775808));
    out.swap = 0 != (num & UINT64_C(4611686018427387904));
    out.fm_sh = 0 != (num & UINT64_C(2305843009213693952));
    out.wprotected = 0 != (num & UINT64_C(144115188075855872));
    out.emapped = 0 != (num & UINT64_C(72057594037927936));
    out.dirty = 0 != (num & UINT64_C(36028797018963968));
    num &= 36028797018963967;

    if (out.active)
        out.rest.frame = num;
    else if (out.swap)
    {
        out.rest.swap_info.type = num & 31;
        out.rest.swap_info.offset = num >> 5;
    }

    return out;
}

void print_pagemap(struct pagemap pagemap, FILE *file)
{
    fwprintf(file, L"active: %d\nswap: %d\n"
                   L"file-mapped page or a shared anonymous: %d\n"
                   L"write-protected: %d\n"
                   L"exclusively mapped: %d\n"
                   L"dirty: %d\n", pagemap.active, pagemap.swap,
                   pagemap.fm_sh, pagemap.wprotected, pagemap.emapped,
                   pagemap.dirty);

    if (pagemap.active)
        fwprintf(file, L"frame number: %llu\n", pagemap.rest.frame);
    else if (pagemap.swap)
        fwprintf(file, L"type: %d\noffset: %llu\n", pagemap.rest.swap_info.type,
                 pagemap.rest.swap_info.offset);
}

int show_pagemap(const size_t n, wchar_t **args, void *arg)
{
    if (1 != n)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    struct file_range range = parse_range(args[0]);

    if (EXIT_FAILURE == range.valid)
        return EXIT_FAILURE;
    else if (WRONG_ARG == range.valid)
        return EXIT_SUCCESS;

    sprintf(file_buf, "/proc/%d/pagemap", *(pid_t *)arg);
    FILE *pagemap_file = fopen(file_buf, "r");

    fseek(pagemap_file, range.start, SEEK_SET);

    for (uint64_t pagemap, cnt = (range.end - range.start) / 8 + 1;
         cnt && 0 < fread(&pagemap, 1, sizeof(uint64_t), pagemap_file);
         cnt--)
    {
        print_bits(pagemap, stdout);
        print_pagemap(get_pagemap(pagemap), stdout);
    }

    fclose(pagemap_file);

    return EXIT_SUCCESS;
}

int show_pagemap_timer(const size_t n, wchar_t **args, void *arg)
{
    if (2 != n && 3 != n)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    struct file_range range = parse_range(args[0]);

    if (EXIT_FAILURE == range.valid)
        return EXIT_FAILURE;
    else if (WRONG_ARG == range.valid)
        return EXIT_SUCCESS;

    wchar_t *tmp;
    unsigned long timeout = wcstoul(args[1], &tmp, 10);

    if ('\0' != *tmp)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    FILE *file = stdout;

    if (3 == n)
    {
        wcstombs(buffer, args[2], BUF_SIZE - 1);
        file = fopen(buffer, "w");

        if (!file)
        {
            wprintf(L"No such file %s", args[2]);
            return EXIT_SUCCESS;
        }
    }

    sprintf(file_buf, "/proc/%d/pagemap", *(pid_t *)arg);
    FILE *pagemap_file = fopen(file_buf, "r");
    run = 1;

    while (run)
    {
        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        sprintf(buffer, "%s", asctime(timeinfo));
        buffer[strlen(buffer) - 1] = '\0';
        fwprintf(file, L"[%s]\n", buffer);

        fseek(pagemap_file, range.start, SEEK_SET);

        for (uint64_t pagemap, cnt = (range.end - range.start) / 8 + 1;
             cnt && 0 < fread(&pagemap, 1, sizeof(uint64_t), pagemap_file);
             cnt--)
            print_bits(pagemap, file);

        sleep(timeout);
    }

    wprintf(L"\n");
    fclose(pagemap_file);

    if (3 == n)
        fclose(file);

    return EXIT_SUCCESS;
}

int show_pagemap_diff(const size_t n, wchar_t **args, void *arg)
{
    if (2 != n && 3 != n)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    struct file_range range = parse_range(args[0]);

    if (EXIT_FAILURE == range.valid)
        return EXIT_FAILURE;
    else if (WRONG_ARG == range.valid)
        return EXIT_SUCCESS;

    wchar_t *tmp;
    unsigned long timeout = wcstoul(args[1], &tmp, 10);

    if ('\0' != *tmp)
    {
        wprintf(L"Wrong args\n");

        return EXIT_SUCCESS;
    }

    FILE *file = stdout;

    if (3 == n)
    {
        wcstombs(buffer, args[2], BUF_SIZE - 1);
        file = fopen(buffer, "w");

        if (!file)
        {
            wprintf(L"No such file %s", args[2]);
            return EXIT_SUCCESS;
        }
    }

    sprintf(file_buf, "/proc/%d/pagemap", *(pid_t *)arg);
    FILE *pagemap_file = fopen(file_buf, "r");
    run = 1;

    const size_t pages = (range.end - range.start) / 8 + 1;
    uint64_t *page_buffer = malloc(pages * sizeof(uint64_t));
    uint64_t start = range.start / 8;
    fseek(pagemap_file, range.start, SEEK_SET);

    for (uint64_t pagemap, cnt = pages;
         cnt && 0 < fread(&pagemap, 1, sizeof(uint64_t), pagemap_file);
         cnt--)
        page_buffer[pages - cnt] = pagemap;

    for (int change = 0; run ; change = 0)
    {
        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        sprintf(buffer, "%s", asctime(timeinfo));
        buffer[strlen(buffer) - 1] = '\0';
        fwprintf(file, L"[%s]\n", buffer);

        fseek(pagemap_file, range.start, SEEK_SET);

        for (uint64_t pagemap, cnt = pages, tmp = 0;
             cnt && 0 < fread(&pagemap, 1, sizeof(uint64_t), pagemap_file);
             cnt--, tmp++)
        {
            if (pagemap != page_buffer[tmp])
            {
                change = 1;
                fwprintf(file, L"%llx\n", (start + tmp) * getpagesize());
                print_bits(page_buffer[tmp], file);
                print_bits(pagemap, file);
            }

            page_buffer[tmp] = pagemap;
        }

        if (!change)
            fwprintf(file, L"No changes\n");

        sleep(timeout);
    }

    wprintf(L"\n");
    fclose(pagemap_file);
    free(page_buffer);

    if (3 == n)
        fclose(file);

    return EXIT_SUCCESS;
}


struct stat
{
    int pid;
    char comm[17];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned int rt_priority;
    unsigned int policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long guest_time;
    long cguest_time;
    unsigned long start_data;
    unsigned long end_data;
    unsigned long start_brk;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long env_start;
    unsigned long env_end;
    int exit_code;
};

int show_stat(const size_t n, wchar_t **args, void *arg)
{
    sprintf(file_buf, "/proc/%d/stat", *(pid_t *)arg);
    FILE *stat_file = fopen(file_buf, "r");

    struct stat stat;

    fscanf(stat_file,
           "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld "
           "%ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
           "%lu %lu %d %d %u %u %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %d",
           &stat.pid, stat.comm, &stat.state, &stat.ppid, &stat.pgrp,
           &stat.session, &stat.tty_nr, &stat.tpgid, &stat.flags, &stat.minflt,
           &stat.cminflt, &stat.majflt, &stat.cmajflt, &stat.utime, &stat.stime,
           &stat.cutime, &stat.cstime, &stat.priority, &stat.nice,
           &stat.num_threads, &stat.itrealvalue, &stat.starttime, &stat.vsize,
           &stat.rss, &stat.rsslim, &stat.startcode, &stat.endcode,
           &stat.startstack, &stat.kstkesp, &stat.kstkeip, &stat.signal,
           &stat.blocked, &stat.sigignore, &stat.sigcatch, &stat.wchan,
           &stat.nswap, &stat.cnswap, &stat.exit_signal, &stat.processor,
           &stat.rt_priority, &stat.policy, &stat.delayacct_blkio_ticks,
           &stat.guest_time, &stat.cguest_time, &stat.start_data,
           &stat.end_data, &stat.start_brk, &stat.arg_start, &stat.arg_end,
           &stat.env_start, &stat.env_end, &stat.exit_code);

    fclose(stat_file);

    wprintf(L"pid: %d\ncomm: %s\nstate: %c\nppid: %d\npgrp: %d\nsession: %d\n"
            L"tty_nr: %d\ntpgid: %d\nflags: %u\nminflt: %lu\ncminflt: %lu\n"
            L"majflt: %lu\ncmajflt: %lu\nutime: %lu\nstime: %lu\ncutime: %ld\n"
            L"cstime: %ld\npriority: %ld\nnice: %ld\nnum_threads: %ld\n"
            L"itrealvalue: %ld\nstarttime: %llu\nvsize: %lu\nrss: %ld\n"
            L"rsslim: %lu\nstartcode: %lu\nendcode: %lu\nstartstack: %lu\n"
            L"kstkesp: %lu\nkstkeip: %lu\nsignal: %lu\nblocked: %lu\n"
            L"sigignore: %lu\nsigcatch: %lu\nwchan: %lu\nnswap: %lu\n"
            L"cnswap: %lu\nexit_signal: %d\nprocessor: %d\nrt_priority: %u\n"
            L"policy: %u\ndelayacct_blkio_ticks: %llu\nguest_time: %lu\n"
            L"cguest_time: %ld\nstart_data: %lu\nend_data: %lu\n"
            L"start_brk: %lu\narg_start: %lu\narg_end: %lu\nenv_start: %lu\n"
            L"env_end: %lu\nexit_code: %d\n",
            stat.pid, stat.comm, stat.state, stat.ppid, stat.pgrp, stat.session,
            stat.tty_nr, stat.tpgid, stat.flags, stat.minflt, stat.cminflt,
            stat.majflt, stat.cmajflt, stat.utime, stat.stime, stat.cutime,
            stat.cstime, stat.priority, stat.nice, stat.num_threads,
            stat.itrealvalue, stat.starttime, stat.vsize, stat.rss, stat.rsslim,
            stat.startcode, stat.endcode, stat.startstack, stat.kstkesp,
            stat.kstkeip, stat.signal, stat.blocked, stat.sigignore,
            stat.sigcatch, stat.wchan, stat.nswap, stat.cnswap,
            stat.exit_signal, stat.processor, stat.rt_priority, stat.policy,
            stat.delayacct_blkio_ticks, stat.guest_time, stat.cguest_time,
            stat.start_data, stat.end_data, stat.start_brk, stat.arg_start,
            stat.arg_end, stat.env_start, stat.env_end, stat.exit_code);

    return EXIT_SUCCESS;
}

int show_task(const size_t n, wchar_t **args, void *arg)
{
    sprintf(file_buf, "/proc/%d/task", *(pid_t *)arg);
    DIR *d = opendir(file_buf);

    for (struct dirent *dir; (dir = readdir(d));)
        wprintf(L"%s\n", dir->d_name);

    closedir(d);

    return EXIT_SUCCESS;
}


int main(void)
{
    setlocale(LC_ALL, "");
    struct sigaction new_action;
    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if (sigaction(SIGINT, &new_action, NULL) == -1)
    {
        perror("sigaction error\n");

        return EXIT_FAILURE;
    }

    pid_t pid = getpid();
    cli_t *cli = cli_init(NULL);

    if (!cli)
        return EXIT_FAILURE;

    cli_set_prompt(cli, prompt, &pid);
    cli_add_entry(cli, L"set", L"Установить pid отслеживаемого процесса", set_pid, &pid);
    cli_add_entry(cli, L"stat", L"Отобразить информацию о процессе", show_stat, &pid);
    cli_add_entry(cli, L"cwd", L"Отобразить полное имя рабочей директории", show_cwd, &pid);
    cli_add_entry(cli, L"environ", L"Отобразить переменные среды", show_env, &pid);
    cli_add_entry(cli, L"task", L"Отобразить поддиректорию потоков", show_task, &pid);
    cli_add_entry(cli, L"maps", L"Отобразить список регионов адресного пространства процесса", show_maps, &pid);
    cli_add_entry(cli, L"pagemap", L"Отобразить диапазон страниц процесса", show_pagemap, &pid);
    cli_add_entry(cli, L"pagemap_timer", L"Отобразить диапазон страниц процесса", show_pagemap_timer, &pid);
    cli_add_entry(cli, L"pagemap_diff", L"Отобразить разницу значений диапазона страниц процесса\n", show_pagemap_diff, &pid);

    cli_mainloop(cli);
    cli_free(&cli);

    return EXIT_SUCCESS;
}

