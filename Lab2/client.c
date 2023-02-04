/* 
 * File:   transfer.c
 * Author: guanyux1, fanweite
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include "transfer.h"
#include "transfer.c"
bool logedIn = false;
bool inSession = false;
bool connected = false;
int serverfd;
int fdmax;
char userName[MAX_DATA];
fd_set master;
fd_set read_fds;
   
int encodeData(char* command[5], char* encodedData);
void handel_server(char* recvbuf);
void handel_user(int dataType, char* command[5], char* encodedData);


int main(int argc, char** argv){
    
    int nbytes;
    char buf[bufsize];
   
    char* command[5];
    char* encodedData;
    encodedData = (char*)malloc(sizeof(char) * MAX_DATA);
    for(int i = 0; i < 5; i++){
        command[i] = (char*)malloc(sizeof(char) * MAX_DATA);
    }
     
    FD_ZERO(&master);
    FD_ZERO(&read_fds);  
    FD_SET(0, &master);
    fdmax = 0;
    printf("Please input command:\n");
    while(1){
        read_fds = master; 
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(0);
        }
                   
        for(int i = 0; i <= fdmax; i++){
            memset(buf, 0, sizeof buf);
            if (FD_ISSET(i, &read_fds)){
                if(i==0){//user input, deal with send message type
                    int dataType = encodeData(command,encodedData);                   
                    handel_user(dataType, command, encodedData);
                }
                else{//receive from server
                    if((nbytes = recv(i, buf, MAX_DATA - 1, 0)) <= 0){
                        // got error or connection closed by server
                        if(nbytes == 0) {
                            // connection closed
                            printf("Server hung up\n");
                        }
                        else{
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else{ // we got some data from a server                      
                        printf("[Caution!] Receive a message form server\n");                               
                        handel_server(buf);//deal with receive message type                 
                    }
                }
            }
        }
    }
}

int encodeData(char* command[5], char* encodedData){
    char input[MAX_DATA];
    scanf("%[^\n]s", input);
    sscanf((char*) input, "%s", (char*) command[0]);
    setbuf(stdin, NULL);
    if (strcmp((char*) command[0], "/login") == 0) {
        sscanf((char*) input, "%s %s %s %s %s", (char*) command[0],(char*) command[1],(char*) command[2],(char*) command[3],(char*) command[4]);
        
        strcpy((char*)encodedData,(char*)command[2]);

        return 1;
    }else if(strcmp((char*) command[0], "/logout") == 0){
        return 4;
    }else if(strcmp((char*)command[0], "/joinsession") == 0){
        sscanf((char*) input, "%s %s", (char*) command[0], (char*) command[1]);
        strcpy((char*) encodedData, (char*) command[1]);
        return 5;
    } else if (strcmp((char*) command[0], "/leavesession") == 0) {
        return 8;
    } else if (strcmp((char*) command[0], "/createsession") == 0) {
        sscanf((char*) input, "%s %s", (char*) command[0], (char*) command[1]);
        strcpy((char*) encodedData, (char*) command[1]);
        return 9;
    } else if (strcmp((char*) command[0], "/list") == 0) {
        return 12;
    } else if (strcmp((char*) command[0], "/quit") == 0) {
        return -1;
    } else {
        strcpy((char*) command[1], (char*) input);
        return 11;
    }
}

void handel_server(char* recvbuf){
    struct message serverRes = readMessage(recvbuf);
    if(serverRes.type == -1){
        printf("See you next time!\n");
        logedIn = false;
        inSession = false;
        memset(userName, 0, sizeof userName);
        FD_CLR(serverfd, &master);
        close(serverfd);
    }else if(serverRes.type == 2){
        printf("%s\n",(char*)serverRes.data);
        printf("Welcome back, %s!\n",userName);
        logedIn = true;
    }else if(serverRes.type == 3){
        printf("Login Fail!\n");
        printf("%s\n",(char*)serverRes.data);
    }else if(serverRes.type == 6){
        printf("%s\n",(char*)serverRes.data);
        inSession = true;
    }else if(serverRes.type == 7){
        printf("Join session Fail!\n");
        printf("%s\n",(char*)serverRes.data);
    }else if(serverRes.type == -2){
        inSession = false;           
        printf("You have left the session, we will miss you!\n"); 
    }else if(serverRes.type == 10){
        printf("%s\n",(char*)serverRes.data);
        inSession = true;
    }else if(serverRes.type == 11){
        printf("%s has sent a message: %s\n", serverRes.source, serverRes.data);
    }else if (serverRes.type == 13) {
        printf("Start printing user list:\n");
        printf("%s\n",(char*)serverRes.data);
    }else{
        printf("Seems like we receive a wrong response from server, please try again!\n");
    }
    printf("Please input command:\n"); 
}

void handel_user(int dataType, char* command[5], char* encodedData){
    struct message userMsg;
    if(dataType == 1){//login
        if(logedIn){
            printf("Already Logged In!\n");
            printf("Please input command:\n");  
        }else{
            struct addrinfo hints;
            struct addrinfo* res;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;  
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            strcpy(userName, (char*)command[1]);

            int rv = getaddrinfo((char *) command[3], (char *) command[4], &hints, &res);
            if(rv != 0){
                printf("Invalid IP address or Port Number!\n");
                printf("Please input command:\n");  
                return;
            }
            serverfd = socket(AF_INET, SOCK_STREAM, 0);                                 
            if (connect(serverfd, res->ai_addr, res->ai_addrlen) == -1) {
                printf("Cannot connect to the Server!\n");
                printf("Please input command:\n");  
                return;
            }
            FD_SET(serverfd, &master);
            fdmax = serverfd;
            userMsg.type = 1;
            userMsg.size = strlen(encodedData);
            strcpy(userMsg.source, (char*) command[1]);
            strcpy(userMsg.data, encodedData);
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");  
            }       
        }
    }else if(dataType == -1){//quit
        if(logedIn){
            userMsg.type = 4;
            userMsg.size = strlen("LogOut");
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, "LogOut");
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n"); 
                return;
            }
            printf("See you next time! Quit program!\n");
            logedIn = false;
            inSession = false;
            memset(userName, 0, sizeof userName); 
            FD_CLR(serverfd, &master);
            close(serverfd);
        }     
        else{
            printf("Quit program!\n");
        }
        exit(0);
    }else if(dataType == 4){//logout
        if(logedIn){
            userMsg.type = 4;
            userMsg.size = strlen("LogOut");
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, "LogOut");
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
                return;
            }
        }else{
            printf("You are not logged in!\n");
            printf("Please input command:\n");
        }
    }else if(dataType == 5){//joinsession
        if(inSession){
            printf("You are already in a session!\n");
            printf("Please input command:\n");
        }else{
            userMsg.type = 5;
            userMsg.size = strlen((char*) command[1]);
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, (char*) command[1]);
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
            }
        }
    }else if(dataType == 8){//leave session
        if(!inSession){
            printf("You are currently not in any session!\n");
            printf("Please input command:\n");
        }else{
            userMsg.type = 8;
            userMsg.size = strlen("LeaveSession");
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, "LeaveSession");
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
            }              
        }
    }else if(dataType == 9){//create session
        if(inSession){
            printf("You are already in a session, please leave the session before create a new one!\n");
            printf("Please input command:\n");
        }else if(atoi((char*)command[1]) > 3 || atoi((char*)command[1]) < 1){
            printf("Invalid session number, session number should be 1, 2 or 3\n");
            printf("Please input command:\n");
        }else{
            userMsg.type = 9;
            userMsg.size = strlen((char*) command[1]);
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, (char*) command[1]);
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
            }             
        }
    }else if(dataType == 11){//send message
        if(!logedIn){
            printf("You have not login, please login first!\n");
            printf("Please input command:\n");
        }
        else if(!inSession){
            printf("You have not join a session yet, please join a session before texting!\n");
            printf("Please input command:\n");
        }else{
            userMsg.type = 11;
            userMsg.size = strlen((char*) command[1]);
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, (char*) command[1]);
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
            }
            printf("Send message successfully!\n");  
            printf("Please input command:\n");  
        }
    }else if(dataType == 12){//list
            userMsg.type = 12;
            userMsg.size = strlen("List");
            strcpy(userMsg.source, userName);
            strcpy(userMsg.data, "List");
            if(sendMsg(serverfd,userMsg) == -1){
                printf("Cannot send message to the Server!\n");
                printf("Please input command:\n");
            }             
    }else{
        printf("Cannot identify the command, please try again!\n");
        printf("Please input command:\n");
    }
}
