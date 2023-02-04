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
    if(argc != 3){
        printf("Not enough or too many arguments\n");
        exit(0);
    }
    char* seraddr = argv[1];
    printf("Seraddr: %s\n", seraddr);
    char* port = argv[2];
    printf("PORT: %s\n", port);
    printf("Please input the message:\n");
    char message[1000];
    char filename[1000];
    scanf("%s %s", message, filename);
    
    if(strcmp(message, "ftp") != 0){
        printf("This is not 'ftp'\n");
        exit(0);
    }else{
        if(access(filename, F_OK ) != -1 ) {
            printf("File exists\n");
        } else {
            printf("File not found\n");
            exit(0);
        }
    }
    
    int sockfd; 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct addrinfo hints;//this code is from Beej
    struct addrinfo* res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    int rv = getaddrinfo(seraddr, port, &hints, &res);
    int n = sendto(sockfd, "ftp", 3, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1){
        printf("Fail to send to server\n");
        exit(0);
    }else {
        printf("Success sending to server\n");
    }
    char buffer[bufsize]; 
    struct sockaddr_storage servaddr; 
    char s_addr[1000]; 
    socklen_t size;
    size = sizeof(servaddr);
    n = recvfrom(sockfd, buffer, bufsize-1, 0, (struct sockaddr *)&servaddr, &size);
    if(n == -1){
        printf("recvform filed\n");
        exit(0);
    }
    buffer[n] = '\0';
  
    if(strcmp(buffer, "yes") == 0){
        printf("A file transfer can start\n");
    }
    else{
        close(sockfd);
        exit(0);
    }
     
    return 0; 
}