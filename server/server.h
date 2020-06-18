#ifndef SERVER_H
#define SERVER_H

#define BUFF_MAX_SIZE       1024         /* size of buff */
#define CMD_MAX_LEN         256          /* command max length */

#ifdef __cplusplus
extern "C" {
#endif

void svr_process(int sockfd, char *buff);
void list(int sockfd, char *buff, int buffLen);
void get(int sockfd, char *buff, int buffLen);

#ifdef __cplusplus
}
#endif

#endif
