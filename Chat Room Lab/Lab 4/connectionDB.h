#ifndef CONNECTION_H
#define CONNECTION_H

#include "initialization.h"

struct Connection {
    char IPaddr[MAX_IPADDR];
    char sessionID[MAX_SESSION_ID];
    int port;
    int socket_fd;
    char clientID[MAX_NAME];
};

struct ConnectionNode {
    struct Connection connection;
    struct ConnectionNode * next;
};

void printConnection(struct Connection * connection);
void printConnectionList(struct ConnectionNode * root);
void addConnection(struct ConnectionNode ** root, struct Connection * data);
int deleteConnectionAtSocket(struct ConnectionNode ** curr_node, int socket);
struct ConnectionNode * findSessionConnections(struct ConnectionNode * root, char * session);
struct ConnectionNode * findClientConnections(struct ConnectionNode * root, char * clientID);
void createConnection(struct Connection * connection, struct sockaddr_in their_addr, char * clientID, char * session, int socket_fd);


#endif
