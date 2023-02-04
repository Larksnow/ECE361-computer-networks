/* 
 * File:   transfer.c
 * Author: guanyux1, fanweite
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "transfer.h"

int sendMsg(int sockfd, struct message userMsg){
    char sendMsg[MAX_DATA];
    sprintf(sendMsg, "%d,%d,%s,%s",userMsg.type, userMsg.size, userMsg.source, userMsg.data);
    return send(sockfd, sendMsg, strlen(sendMsg), 0);
}

struct message readMessage(char* incomingMsg){
    struct message decodedMsg;
    unsigned char source_data[MAX_DATA];
    sscanf(incomingMsg, "%d,%d,%[^\n]s", &decodedMsg.type, &decodedMsg.size, source_data);
     // split soruce and data
    char* comma;
    comma = strchr ((char*) source_data, ',');
    *comma = '\0';
    comma += sizeof(unsigned char);
    strcpy((char*) decodedMsg.source, (char*) source_data);

    strcpy((char*) decodedMsg.data, (char*) comma);
    
    return decodedMsg;
}