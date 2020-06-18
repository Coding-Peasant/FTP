#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>

#define BUFF_MAX_SIZE       1024     /* size of buff */
#define CMD_MAX_LEN    256     /* command max length */

#ifdef __cplusplus
extern "C" {
#endif

void list(struct sockaddr_in addr, char *cmd);
void get(struct sockaddr_in addr, char *cmd);

#ifdef __cplusplus
}
#endif

#endif
