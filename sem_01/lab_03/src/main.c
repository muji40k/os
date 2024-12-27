#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include "daemon.h"

sigset_t mask;

void change_wallpaper(void)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        syslog(LOG_ERR, "Ошибка fork change_wallpaper");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        if ((execl("/bin/bash", "bash", "-c", "nitrogen --set-zoom-fill --random /usr/share/backgrounds/")) == -1)
            syslog(LOG_ERR, "Невозможно изменить обои");
        exit(EXIT_FAILURE);
    }
}

void *thr_fn(void *arg)
{
    int err, signo;
    for (;;)
    {
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "Ошибка вызова функции sigwait");
            exit(EXIT_FAILURE);
        }
        switch (signo)
        {
            case SIGHUP:
                syslog(LOG_INFO, "Изменеие обоев рабочего стола");
                change_wallpaper();
                break;
            case SIGTERM:
                syslog(LOG_INFO, "Получен сигнал SIGTERM - Выход");
                exit(EXIT_SUCCESS);
            default:
                syslog(LOG_INFO, "Получен непредвиденный сигнал %d\n", signo);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        perror("Wrong usage\n");
        exit(EXIT_FAILURE);
    }
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;
    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    // Перейти в режим демона.
    daemonize(cmd);
    // Убедиться, что ранее не была запущена другая копия демона.
    if (already_running())
    {
        syslog(LOG_ERR, "Демон уже запущен");
        exit(EXIT_FAILURE);
    }
    // Восстановить действие по умолчанию для сигнала SIGHUP
    // и заблокировать все сигналы.
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        syslog(LOG_ERR, "Невозможно восстановить действие SIG_DFL для SIGHUP");
        exit(EXIT_FAILURE);
    }
    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        syslog(LOG_ERR, "Ошибка выполнения операции SIG_BLOCK");
        exit(EXIT_FAILURE);
    }
    // Создать поток для обработки SIGHUP и SIGTERM.
    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
    {
        syslog(LOG_ERR, "Невозможно создать поток");
        exit(EXIT_FAILURE);
    }
    // Остальная часть программы-демона.
    for (time_t tm; 1;)
    {
        sleep(5);
        tm = time(NULL);
        syslog(LOG_INFO, "%s", asctime(localtime(&tm)));
    }
    return EXIT_SUCCESS;
}

