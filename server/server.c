#include "server.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char **argv){
    int listen_sockfd, accept_sockfd;
    struct sockaddr_in svr_addr, cli_addr;
    const char *port = "20021";              /* default port */
    int len = sizeof(cli_addr);
    char buff[BUFF_MAX_SIZE] = {0};
    int pid;

    memset(&svr_addr, 0, sizeof(svr_addr));

    if(2 == argc){
        port = argv[1];
    }

    /* create socket */
    if ( (listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket fail.\n");
        return -1;
    }

    /* set local address info */
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(atoi(port));

    /* set socket option */
    //setsocketopt();

    /* bind */
    if (bind(listen_sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) < 0){
        close(listen_sockfd);
		perror("bind() error");
		return -1; 
    }

    /* listening */
	if (listen(listen_sockfd, 10) < 0) {
		close(listen_sockfd);
		perror("listen() error");
		return -1;
	}   

    /* wait for clinet request */
    while(1){

        /* create new socket for client request */
        accept_sockfd = accept(listen_sockfd, (struct sockaddr *) &cli_addr, &len);
        if (accept_sockfd < 0){
            perror("accept() error"); 
            return -1; 
        }

        /* concurrent processing */
        if((pid = fork()) < 0){
            perror("Error forking child process");

        }else if(0 == pid) {    /* child process */
            /* close listening socket */
            close(listen_sockfd);

            /* do something */
            svr_process(accept_sockfd, buff);

            /* close the socket for client request */
            close(accept_sockfd);
            exit(0);
        }
        /* parent close the socket for client request */
        close(accept_sockfd);
    }

    /* close listening socket */
    close(listen_sockfd);

    return 0;  
}

void svr_process(int sockfd, char *buff){
    int len;
    memset(buff, 0, BUFF_MAX_SIZE);
    if(recv(sockfd, buff, BUFF_MAX_SIZE, 0) < 0){
        perror("accept() error"); 
        exit(1); 
    }

    memcpy(&len, buff, sizeof(len));

    printf("receive cmd[ %s ] and length[ %d ].\n", buff+4, len);
    
    /* parse command */
    if(strncmp(buff+4, "list", 4) == 0){
        list(sockfd, buff, len);
    }else if (strncmp(buff+4, "get", 3) == 0){
        get(sockfd, buff, len);  
    }else {
       printf("Command '%s' not found!\n", buff+4); 
    }
}

void list(int sockfd, char *buff, int len){
    DIR *dir = NULL;
    struct dirent *item = NULL;
    char cmd[CMD_MAX_LEN] = {0};
    int length;
    memcpy(cmd, buff+4, len);

    if((dir = opendir(cmd+5)) == NULL){
        perror("opendir() error"); 
        exit(1); 
    }

    while((item = readdir(dir)) != NULL){
        if(sprintf(buff+4, item->d_name, BUFF_MAX_SIZE-4) < 0){
            perror("sprintf() error"); 
            exit(1); 
        }

        length = strlen(buff+4)+1;
        memcpy(buff, &length, sizeof(length));
    
        /* send msg to client */
        if( send(sockfd, buff, BUFF_MAX_SIZE, 0) < 0){
            perror("send() error.\n");
            exit(1);
        }
        memset(buff, 0, BUFF_MAX_SIZE);
    }
    closedir(dir);
}

void get(int sockfd, char *buff, int len){
    FILE *fd = NULL;
    char cmd[CMD_MAX_LEN] = {0};
    struct stat statbuf;
    int reads = 0;
    int sum_reads = 0;
    int length;
    memcpy(cmd, buff+4, len);
    stat(cmd+4, &statbuf);
    length = statbuf.st_size;
    memcpy(buff, &length, sizeof(length));

    if((fd = fopen(cmd+4, "r")) == NULL){
        perror("fopen() error"); 
        exit(1); 
    }

    /* send file to client */
    if((BUFF_MAX_SIZE - sizeof(length)) > length){
        reads = fread(buff+sizeof(length), 1, length, fd);
        if( send(sockfd, buff, reads, 0) < 0){
            perror("send() error.\n");
            exit(1);
        }
        sum_reads +=reads;

    }else{
        reads = fread(buff+sizeof(length), 1, BUFF_MAX_SIZE-sizeof(length), fd);
        if( send(sockfd, buff,  BUFF_MAX_SIZE, 0) < 0){
            perror("send() error.\n");
            exit(1);
        }
        sum_reads +=reads;
        memset(buff, 0, BUFF_MAX_SIZE);

        while (sum_reads < length){
            reads = length - sum_reads > BUFF_MAX_SIZE ? BUFF_MAX_SIZE : length - sum_reads;
            reads = fread(buff, 1, reads, fd);
            if( send(sockfd, buff,  reads, 0) < 0){
                perror("send() error.\n");
                exit(1);
            }
            sum_reads += reads;
            memset(buff, 0, BUFF_MAX_SIZE);
        }
    }

    fclose(fd);  
}