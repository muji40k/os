#ifndef _SOCKET_SETUP_H_
#define _SOCKET_SETUP_H_

#include <sys/socket.h>
#include <unistd.h>

struct sockaddr setup_server(void);
struct sockaddr setup_client(const char *const name);

void clean_socket(struct sockaddr *const addr);

#endif

