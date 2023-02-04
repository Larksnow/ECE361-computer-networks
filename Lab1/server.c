#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#define bufsize 1000

int main(int argc, char** argv){
    while(1){
        
        if(argc != 2){
            printf("Not enough or too many arguments.\n");
            exit(0);
        }
        char* port = argv[1];  
        printf("PORT: %s\n", port);
        char buffer[bufsize]; 

        struct addrinfo hints;//this code is from Beej
        struct addrinfo* res;
        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        int rv = getaddrinfo(NULL, port, &hints, &res);//argument1 is internet host argument 2 is service
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);//create the socket

        bind(sockfd, res->ai_addr, res->ai_addrlen);//bind
        printf("waiting to recvfrom\n");

        struct sockaddr_storage cliaddr;

        socklen_t size;
        size = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, bufsize-1, 0, (struct sockaddr *)&cliaddr, &size);
        if(n == -1){
            printf("recvform filed\n");
            exit(0);
        }
        else{
            printf("recvform success\n");
        }
        buffer[n] = '\0';

        if(strcmp(buffer, "ftp") == 0){
            printf("Ftp is requested\n");
            n = sendto(sockfd, "yes", 3, 0, (struct sockaddr *)&cliaddr, size);
        }else{
            printf("request is %s\n", buffer);
            n = sendto(sockfd, "no", 2, 0, (struct sockaddr *)&cliaddr, size);
        }
        if(n == -1){
            printf("fail to send feedback to server\n");
            exit(0);
        }
        else{
            printf("success to send feedback to server\n"); 
        }
    }
}