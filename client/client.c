#include "client.h"
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<unistd.h>

int main(int argc, char **argv){
    struct sockaddr_in addr;
    const char *host = "127.0.0.1";    /* default host */
    const char *port = "20021";        /* default port */
    char cmd[CMD_MAX_LEN];
    char *nl = NULL;

    memset(&addr, 0, sizeof(addr));

    if(3 == argc){
        host = argv[1];
        port = argv[2];
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(atoi(port));

    /* get server address */
    //getaddreinfo();

    /* free address info */
    //freeaddrinfo();

    /* loop until user types quit */
    while(1){

        /* get a command from user */
        if(NULL == fgets(cmd, CMD_MAX_LEN, stdin)){
            printf("get command fail.\n");
            continue;
        }

        /* replace '\n' with '\0' */
        nl = strchr(cmd, '\n');
		if (nl) *nl = '\0'; 

        /* parse command */
        if(strncmp(cmd, "help", 4) == 0){
            printf("help.\n");
        }else if (strncmp(cmd, "list", 4) == 0){
            list(addr, cmd);
        }else if (strncmp(cmd, "get", 3) == 0){
            get(addr, cmd);
        }else if( strncmp(cmd, "quit", 4) == 0){
            printf("Quit simple FTP.\n");
            break;
        }else {
            printf("Command '%s' not found!\n", cmd);
        }
    }

    return 0;
}

void list(struct sockaddr_in addr, char *cmd){
    int sockfd;
    char buff[BUFF_MAX_SIZE] = {0};
    int len = strlen(cmd) + 1;
    memcpy(buff+4, cmd, len);
    memcpy(buff, &len, sizeof(len));
    int total = 0;
    
    /* create socket */
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket fail.\n");
        exit(1);
    }

    /* connect server*/
    if( connect(sockfd, &addr, sizeof(addr)) < 0){
        printf("connect server fail.\n");
        exit(1);
    }

    /* send command to server */
    if( send(sockfd, buff, BUFF_MAX_SIZE, 0) < 0){
        printf("send msg fail.\n");
        exit(1);
    }

    memset(buff, 0, BUFF_MAX_SIZE);

    while( recv(sockfd, buff, BUFF_MAX_SIZE, 0) > 0){
        total++;
        printf("%s\n", buff+4);
		memset(buff, 0, BUFF_MAX_SIZE);
    }
    printf("There are %d files.\n", total);
    close(sockfd);
}

void get(struct sockaddr_in addr, char *cmd){
    int sockfd;
    FILE *fd;
    int writes = 0;
    int sum_writes = 0;
    char buff[BUFF_MAX_SIZE] = {0};
    int len = strlen(cmd) + 1;
    memcpy(buff+4, cmd, len);
    memcpy(buff, &len, sizeof(len));
    
    /* create socket */
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket fail.\n");
        exit(1);
    }

    /* connect server*/
    if( connect(sockfd, &addr, sizeof(addr)) < 0){
        printf("connect server fail.\n");
        exit(1);
    }

    /* send command to server */
    if( send(sockfd, buff, BUFF_MAX_SIZE, 0) < 0){
        printf("send msg fail.\n");
        exit(1);
    }

    memset(buff, 0, BUFF_MAX_SIZE);
    if((writes = recv(sockfd, buff, BUFF_MAX_SIZE, 0)) < 0){
        printf("receive msg fail.\n");
        exit(1);
    }
    memcpy(&len, buff, 4);
    fd = fopen(cmd+4, "w");
 
    if((BUFF_MAX_SIZE - sizeof(len)) > len){
        writes = fwrite(buff+4, 1, len, fd);
        sum_writes +=writes;

    }else{
        writes = fwrite(buff+sizeof(len), 1, BUFF_MAX_SIZE-sizeof(len), fd);
        sum_writes +=writes;
        memset(buff, 0, BUFF_MAX_SIZE);

        while (sum_writes < len){
            writes = len - sum_writes > BUFF_MAX_SIZE ? BUFF_MAX_SIZE : len - sum_writes;
            if((writes = recv(sockfd, buff, writes, 0)) < 0){
                printf("receive msg fail.\n");
                exit(1);
            }
            writes = fwrite(buff, 1, writes, fd);
            sum_writes += writes;
            memset(buff, 0, BUFF_MAX_SIZE);
        }
    }
    fsync(fd);

    printf("\n");
    fclose(fd);  
    close(sockfd);
}