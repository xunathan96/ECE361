#ifndef CLIENTDB_H
#define CLIENTDB_H

#include "initialization.h"

#define CLIENT_DB "client_db.txt"

struct client {
    char clientID[MAX_NAME];
    char password[MAX_PASSWORD];
	int sockfd;
};

struct clientNode {
    struct client client;
    struct clientNode * next;
};

struct client * find_client(char * clientID, char * filename);

int searchClientList(struct clientNode * clientList, char * clientID);
int removeFromClientList(struct clientNode ** curr_node, char * clientID);
void addToClientList(struct clientNode ** root, struct client * data);

void printClient(struct client * client);
void printClientList(struct clientNode * root);


int removeFromClientListBySock(struct clientNode ** curr_node, int sockfd);
#endif
