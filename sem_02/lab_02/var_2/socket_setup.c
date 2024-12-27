#include "socket_setup.h"
#include <string.h>

#define SOCK_NAME_SERVER "lab_sock_srv"

static struct sockaddr setup(const char *const name)
{
    struct sockaddr out;
    out.sa_family = AF_UNIX;
    strncpy(out.sa_data, name, sizeof(out.sa_data));
    return out;
}

struct sockaddr setup_server(void)
{
    return setup(SOCK_NAME_SERVER);
}

struct sockaddr setup_client(const char *const name)
{
    return setup(name);
}

void clean_socket(struct sockaddr *const addr)
{
    unlink(addr->sa_data);
}

