#include "daemon.h"

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    // Сбросить маску режима создания файла.
    umask(0);
    // Получить максимально возможный номер дескриптора файла.
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
    {
        perror("Невозможно получить максимальный номер дескриптора\n");
        exit(EXIT_FAILURE);
    }
    // Стать лидером нового сеанса, чтобы утратить управляющий терминал.
    if ((pid = fork()) == -1)
    {
        perror("Ошибка вызова функции fork\n");
        exit(EXIT_FAILURE);
    }
    else if (pid != 0)
        exit(EXIT_SUCCESS);
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("Невозможно игнорировать сигнал SIGHUP\n");
        exit(EXIT_FAILURE);
    }
    setsid();
    // Инициализировать файл журнала.
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    // Назначить корневой каталог текущим рабочим каталогом,
    // чтобы впоследствии можно было отмонтировать файловую систему.
    if (chdir("/") == -1)
    {
        syslog(LOG_ERR, "Невозможно сделать текущим рабочим каталогом /\n");
        exit(EXIT_FAILURE);
    }
    // Закрыть все открытые файловые дескрипторы.
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (unsigned long i = 0; i < rl.rlim_max; i++)
        close(i);
    // Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "Ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(EXIT_FAILURE);
    }
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}

int already_running(void)
{
    char buf[16];
    const char *const filename = "/var/run/lab_daemon.pid";
    const int perms = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    int fd = open(filename, O_RDWR|O_CREAT, perms);
    if (fd == -1)
    {
        syslog(LOG_ERR, "Невозможно открыть %s: %s", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return EXIT_FAILURE;
        }
        syslog(LOG_ERR, "Невозможно установить блокировку на %s: %s", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return EXIT_SUCCESS;
}

