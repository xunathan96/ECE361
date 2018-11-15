#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "initialization.h"
#include "message.h"
#include "clientDB.h"
#include "connectionDB.h"

// PACKET TYPES -----------------------

#define LOGIN 10
#define LO_ACK 11
#define LO_NACK 12

#define EXIT 20

#define JOIN 30
#define JN_ACK 31
#define JN_NACK 32

#define LEAVE_SESS 40
#define LS_ACK 41
#define LS_NACK 42

#define NEW_SESS 50
#define NS_ACK 51
#define NS_NACK 52

#define MESSAGE 60
#define MESSAGE_ACK 61
#define MESSAGE_NACK 62

#define QUERY 70
#define QU_ACK 71

#define UNKNOWN_COMMAND 80

// -------------------------------------

#define SEND_ACK 1
#define SEND_NACK 2
#define LOGIN_FAIL 3
#define MULTICAST 4
#define CLOSE 0


int eventHandler(struct message * message, char * response, int socket_fd, struct ConnectionNode ** connectionList, struct clientNode ** clientList);

int unicast(int sendto_socket, char * message);
int multicast(struct ConnectionNode * sessionList, char * message, int except_socket);

void serializeConnectionList(struct ConnectionNode * connectionList, char * serializedList);

#endif
