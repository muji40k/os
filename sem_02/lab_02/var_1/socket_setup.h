#ifndef _SOCKET_SETUP_H_
#define _SOCKET_SETUP_H_

#include <sys/socket.h>
#include <unistd.h>

struct sockaddr setup_server(void);

void clean_socket(struct sockaddr *const addr);

#endif

