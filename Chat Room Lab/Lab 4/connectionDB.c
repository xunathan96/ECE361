#include "connectionDB.h"

void printConnection(struct Connection * connection){
    printf("------------------------------------\n");
    printf(" Client ID: %s\n", connection->clientID);
    printf("IP address: %s\n", connection->IPaddr);
    printf("      Port: %d\n", connection->port);
    printf(" Socket FD: %d\n", connection->socket_fd);
    printf("Session ID: %s\n", connection->sessionID);
}

void printConnectionList(struct ConnectionNode * root){
    struct ConnectionNode * curr_node = root;
    while(curr_node != NULL){
        printConnection(&curr_node->connection);
        curr_node = curr_node->next;
    }
}


void addConnection(struct ConnectionNode ** root, struct Connection * data){
    struct ConnectionNode * new_node = malloc(sizeof(struct ConnectionNode));
    new_node->connection = *data; //must copy data here! shallow copy works
    new_node->next = NULL;

    if(*root==NULL){
        *root = new_node;
    }
    else{
        new_node->next = *root;
        *root = new_node;
    }
}


int deleteConnectionAtSocket(struct ConnectionNode ** curr_node, int socket){
    if(*curr_node==NULL){
        return 0;
    }
    if((*curr_node)->connection.socket_fd == socket){
        struct ConnectionNode * deleted = *curr_node;
        *curr_node = (*curr_node)->next;
        free(deleted);
        return 1;
    }
    else{
        return deleteConnectionAtSocket(&((*curr_node)->next), socket);
    }
}





//Returns a list of connections with specified SessionID
struct ConnectionNode * findSessionConnections(struct ConnectionNode * root, char * session){
    struct ConnectionNode * sessionList = NULL;
    struct ConnectionNode * curr_node = root;

    while(curr_node!=NULL){
        if(strcmp(curr_node->connection.sessionID, session)==0){
            addConnection(&sessionList, &curr_node->connection);
        }
        curr_node = curr_node->next;
    }
    return sessionList;
}

//Returns a list of connections made by specified ClientID
struct ConnectionNode * findClientConnections(struct ConnectionNode * root, char * clientID){
    struct ConnectionNode * clientConnections = NULL;
    struct ConnectionNode * curr_node = root;

    while(curr_node!=NULL){
        if(strcmp(curr_node->connection.clientID, clientID)==0){
            addConnection(&clientConnections, &curr_node->connection);
        }
        curr_node = curr_node->next;
    }
    return clientConnections;
}


void createConnection(struct Connection * connection, struct sockaddr_in their_addr, char * clientID, char * session, int socket_fd){
    strcpy(connection->clientID, clientID);
    strcpy(connection->IPaddr, inet_ntoa(their_addr.sin_addr));
    strcpy(connection->sessionID, session);
    connection->port = ntohs(their_addr.sin_port);
    connection->socket_fd = socket_fd;
}


/*
int main(){


    struct ConnectionNode * root;
    struct ConnectionNode * sessionList;


    struct Connection new_connection;

    strcpy(new_connection.IPaddr, "127.0.0.1");
    strcpy(new_connection.sessionID, "session 1");
    new_connection.port = 3490;
    new_connection.socket_fd = 4;
    addConnection(&root, &new_connection);

    strcpy(new_connection.IPaddr, "127.0.0.2");
    strcpy(new_connection.sessionID, "session 2");
    new_connection.port = 3491;
    new_connection.socket_fd = 5;
    addConnection(&root, &new_connection);

    strcpy(new_connection.IPaddr, "127.0.0.3");
    strcpy(new_connection.sessionID, "session 3");
    new_connection.port = 3492;
    new_connection.socket_fd = 6;
    addConnection(&root, &new_connection);

    strcpy(new_connection.IPaddr, "127.0.0.4");
    strcpy(new_connection.sessionID, "session 4");
    new_connection.port = 3493;
    new_connection.socket_fd = 7;
    addConnection(&root, &new_connection);

    strcpy(new_connection.IPaddr, "127.0.1.1");
    strcpy(new_connection.sessionID, "session 2");
    new_connection.port = 3494;
    new_connection.socket_fd = 8;
    addConnection(&root, &new_connection);

    strcpy(new_connection.IPaddr, "127.0.1.2");
    strcpy(new_connection.sessionID, "session 2");
    new_connection.port = 3495;
    new_connection.socket_fd = 9;
    addConnection(&root, &new_connection);

    printList(root);

    //printf("\nDELETE CONNECTION AT SOCKET\n");
    //deleteConnectionAtSocket(&root, 7);
    //printList(root);

    printf("\n\nSESSION LIST\n");

    sessionList = findSessionConnections(root, "session 2");

    printList(sessionList);


    return 0;
}
*/
