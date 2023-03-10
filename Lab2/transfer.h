/* 
 * File:   transfer.c
 * Author: guanyux1, fanweite
 *
 */
#ifndef MES_H
#define MES_H
#define MAX_NAME 1000
#define MAX_DATA 1000
#define bufsize 1000
typedef enum {false, true} bool;

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

int sendMessage(int s, struct message encodedMsg);
struct message readMessage(char* incomingMsg);

// message types
/*
   -2: [server -> client] acknowledge leave session
   -1: [server -> client] acknowledge logout
    1: [client -> server] login
        <clientID, clientPW>
    2: [server -> client] acknowledge successful login
    3: [server -> client] negative acknowledgement successful login
        <failureMsg>
    4: [client -> server] exit from the server
    5: [client -> server] join a session
        <sessionID>
    6: [server -> client] acknowledge successful join
        <sessionID>
    7: [server -> client] acknowledge unsuccessful join
        <sessionID, failureMsg>
    8: [client -> server] leave session
    9: [client -> server] create new session and join
        <sessionID>
    10:  [server -> client] acknowledge new session
    11:  [client -> server] send message in joined session
        <msg>
    12:  [client -> server] query for list of online users and avaliable sessions
    13:  [server -> client] reply query in message type 12
        <users and sessions list>
*/
#endif