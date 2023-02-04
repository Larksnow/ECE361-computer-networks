/* 
 * File:   transfer.c
 * Author: guanyux1, fanweite
 *
 */
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
#include "transfer.h"
#include "transfer.c"
#define BACKLOG 10

struct userinfo{
    bool active;
    bool insession;
    char name[100];
    char ip[100];
    char pwd[100];
    int sessionid;
    int port;
    int sid;
    struct sockaddr addr;
};

struct sessioninfo{
    int number;
    bool vaild;
};

struct userinfo userlist[5];
struct sessioninfo sessionlist[3];
void initialdata();
bool checklogin(char* username, char*password, char* retmsg, struct sockaddr caddr, int sockid);
int finduser(char* username);
void logout(char* username);
bool joinsession(char* username, int sessionid, char* ackinfo);
void leavesession(char* username);
void createsession(char* username, int sessionid, char* ackinfo);
bool sendmessage(char* username, struct message sendmsg);
void createlist(char* ackinfo);
int handel_income(struct sockaddr caddr, char* inmsg, char* ackinfo, char* source, int sockid);

void initialdata(){

    for(int i=0; i<5; ++i){
        
        userlist[i].active = false;
        userlist[i].insession = false;
        userlist[i].sessionid = -1;
    }
    strcpy((char*)userlist[0].name, "A");
    strcpy((char*)userlist[0].pwd, "000");
    strcpy((char*)userlist[1].name, "B");
    strcpy((char*)userlist[1].pwd, "111");
    strcpy((char*)userlist[2].name, "C");
    strcpy((char*)userlist[2].pwd, "222");
    strcpy((char*)userlist[3].name, "D");
    strcpy((char*)userlist[3].pwd, "333");
    strcpy((char*)userlist[4].name, "E");
    strcpy((char*)userlist[4].pwd, "444");
   
    for(int j=0; j<3; ++j){
        sessionlist[j].number = 0;
        sessionlist[j].vaild = false;
    }
}

bool checklogin(char* username, char*password, char* retmsg, struct sockaddr caddr, int sockid){
    for(int i=0; i<5; ++i){
        if(strcmp((char*) username, (char*) userlist[i].name) == 0){
             if(userlist[i].active){
                 strcpy((char*) retmsg, "User already logged in");
                 return false;
             }
             else{
                 if(strcmp((char*) password, (char*) userlist[i].pwd) == 0){
                     strcpy((char*) retmsg, "Successfully logged in");
                     userlist[i].active = true;
                     userlist[i].addr = caddr;
                     userlist[i].sid = sockid; 
                     struct sockaddr_in *addr_in = (struct sockaddr_in *)&caddr;
                     char *s = inet_ntoa(addr_in->sin_addr);
                     int portNum = addr_in->sin_port;
                     userlist[i].port= portNum;

                     char portnum[10];
                     sprintf(portnum, "%d", portNum);
                     strcpy((char*)userlist[i].ip, (char*) s);
                     printf("user port is %d, user ip is %s\n", userlist[i].port, userlist[i].ip);
                     return true;
                 }
                 else{
                     strcpy((char*) retmsg, "Password wrong");
                     return false;
                 }
             }
        }
    }
    strcpy((char*) retmsg, "Username not found");
    return false;
}

int finduser(char* username){
    for(int i=0; i<5; ++i){
        if(strcmp((char*) username, (char*) userlist[i].name) == 0){
            return i;
        }
    }
}

void logout(char* username){
    int index = finduser(username);
    userlist[index].active = false;
    if(userlist[index].insession = true){
        leavesession(username);
    }
}

bool joinsession(char* username, int sessionid, char* ackinfo){
    int index = finduser(username);
    if(sessionid > 3 || sessionid < 1){
        strcpy((char*)ackinfo, "Invalid session ID");
        return false;
    }
    if(sessionlist[sessionid-1].vaild == true){
        userlist[index].insession = true;
        userlist[index].sessionid = sessionid;
        sessionlist[sessionid-1].number++;
        strcpy((char*)ackinfo, "Successfully join session");
        return true;
    }
    else{
        strcpy((char*)ackinfo, "Session has not been created");
        return false;
    }    
}

void leavesession(char* username){
    int index = finduser(username);
    int sindex = userlist[index].sessionid-1;
    sessionlist[sindex].number--;
    if(sessionlist[sindex].number == 0 ){
        sessionlist[sindex].vaild = false;
    }
    userlist[index].sessionid = -1;
    userlist[index].insession = false;
}

void createsession(char* username, int sessionid, char* ackinfo){
    int index = finduser(username);
    sessionlist[sessionid-1].vaild = true;
    joinsession(username, sessionid, ackinfo);
    strcpy((char*)ackinfo, "Successfully create and join the session");
}

bool sendmessage(char* username, struct message sendmsg){
    int index = finduser(username);
    int session = userlist[index].sessionid;
    for(int i=0; i<5; ++i){
        if(userlist[i].sessionid == session){//if user is in the same session
            if(userlist[i].sid != userlist[index].sid){//send msg except sender itself
                if(sendMsg(userlist[i].sid, sendmsg) == -1){
                    printf("Failed to send text to client %s\n", userlist[i].name);
                }
            }    
        }
    }
    printf("Sending finished\n");
    return true;
}

void createlist(char* ackinfo){
    char list[MAX_DATA];
    memset(list, 0, sizeof list);
    for(int i=0; i<5; ++i){
        if(userlist[i].active){
            strcat((char*) list, "User ");
            strcat((char*) list, (char*)userlist[i].name);
            strcat((char*) list, " ");
            if(userlist[i].insession){
                strcat((char*) list, "is in session ");
                int session = userlist[i].sessionid;
                char str[MAX_DATA];
                sprintf(str, "%d, ", session);
                strcat((char*) list, (char*) str);
            }
        }
    }
    strcpy((char*)ackinfo, (char*) list);
}

int handel_income(struct sockaddr caddr, char* inmsg, char* ackinfo, char* source, int sockid){
    struct message decode = readMessage(inmsg);
    char username[MAX_DATA];
    strcpy((char*)username, (char*) decode.source);
    if(decode.type != 11){
         strcpy((char*)source, "SERVER");
    }
    if(decode.type == 1){
        char password[MAX_DATA];
        strcpy((char*)password, (char*) decode.data);
        if(checklogin(username, password, ackinfo, caddr, sockid)){
            return 2;
        }
        else{
            return 3;
        }
    }
    if(decode.type == 4){
        logout(username);
        return -1;
    }
    if(decode.type == 5){
        char session[MAX_DATA];
        strcpy((char*)session, (char*) decode.data);     
        if(joinsession(username, atoi(session), ackinfo)){
            return 6;
        }
        else{
            return 7;
        }
    }
    if(decode.type == 8){
        leavesession(username);
        return -2;
    }
    if(decode.type == 9){
        char session[MAX_DATA];
        strcpy((char*)session, (char*) decode.data);
        createsession(username, atoi(session), ackinfo);
        return 10;
    }
    if(decode.type == 11){
        sendmessage(username, decode);
        return 11;
    }
    if(decode.type == 12){
        createlist(ackinfo);
        return 13;
    }
}


int main(int argc, char** argv){
    
    if(argc != 2){
        printf("Not enough or too many arguments.\n");
        exit(0);
    }
    initialdata();
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int newfd;
    int nbytes;
    char buf[bufsize];
    
    char* port = argv[1];
    printf("port is: %s\n", port);
    // socket()
    struct addrinfo hints;
    struct addrinfo* res;
    int yes=1;
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    

    int rv = getaddrinfo(NULL, port, &hints, &res);

    int listener = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(listener, res->ai_addr, res->ai_addrlen);

    if(listen(listener, BACKLOG) == -1){
        printf("listen\n");
        exit(0);
    }

    printf("server: waiting for connections...\n");
    FD_SET(listener, &master);
    fdmax = listener;
    // start transmission
    struct sockaddr their_addr; // connector's address information
    socklen_t addrlen;
   
    while (1) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(0);
        }    
        // run through the existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++) {
            memset(buf, 0, sizeof buf);
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof their_addr;
                    newfd = accept(listener,(struct sockaddr *)&their_addr,&addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } 
                    else{
                        FD_SET(newfd, &master); // add to master set
                        if(newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from socket %d\n",newfd);
                    }
                }
                else{
                    // handle data from a client
                    if((nbytes = recv(i, buf, sizeof buf, 0)) <= 0){
                        // got error or connection closed by client
                        if(nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else{
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } 
                    else{
                        // we got some data from a client
                        printf("[Caution!]Receive a message form a client\n");
                        struct message msend;
                        memset(msend.data, 0, sizeof MAX_DATA);
                        msend.type = handel_income(their_addr, buf, msend.data, msend.source, i);
                        msend.size = strlen((char*)msend.data);                                      
                        if(msend.type != 11){
                            int send = sendMsg(i, msend); 
                            if(send == -1){
                            printf("failed to send ack back\n");
                            }
                            printf("ACK back to client\n"); 
                        }                     
                    }
                } // END handle data from client
            } // END got new incoming connection
        }
    }
    return 0;
}     


       
     