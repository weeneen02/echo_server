#ifndef __SERVER_H__
#define __SERVER_H__
#include "common.h"

int run_server(void);
int echo_message(FILE* log_fd, int fd, struct sockaddr_in client_addr);
void config_server_addr(struct sockaddr_in* my_addr);
#endif
