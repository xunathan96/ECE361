#include "clientDB.h"

struct client * find_client(char * clientID, char * filename){
    struct client * client;
    char userID[MAX_NAME];
    char userPassword[MAX_PASSWORD];

    FILE * fp;
    fp = fopen(filename, "r");

    int r = 1;
    while(r!=EOF){
        r = fscanf(fp, "%s %s\n", userID, userPassword);
        if(strcmp(userID, clientID) == 0){
            client = malloc(sizeof(struct client));
            strcpy(client->clientID, userID);
            strcpy(client->password, userPassword);
            fclose(fp);
            return client;
        }
    }
    fclose(fp);
    return NULL;
}

int searchClientList(struct clientNode * clientList, char * clientID){
    struct clientNode * curr_node = clientList;
    while (curr_node!=NULL) {
        if(strcmp(curr_node->client.clientID, clientID)==0){
            return 1;   //CLIENT IS LOGGED IN
        }
        curr_node = curr_node->next;
    }
    return 0;   //CLIENT IS NOT LOGGED IN
}

int removeFromClientList(struct clientNode ** curr_node, char * clientID){
    if(*curr_node==NULL){
        return 0;
    }

    if(strcmp((*curr_node)->client.clientID, clientID)==0){
        struct clientNode * deleted = *curr_node;
        *curr_node = (*curr_node)->next;
        free(deleted);
        return 1;
    }
    else{
        return removeFromClientList(&((*curr_node)->next), clientID);
    }
}

int removeFromClientListBySock(struct clientNode ** curr_node, int sockfd){
    if(*curr_node==NULL){
        return 0;
    }

    if((*curr_node)->client.sockfd == sockfd){
        struct clientNode * deleted = *curr_node;
        *curr_node = (*curr_node)->next;
        free(deleted);
        return 1;
    }
    else{
        return removeFromClientListBySock(&((*curr_node)->next), sockfd);
    }
}

void addToClientList(struct clientNode ** root, struct client * data){
    struct clientNode * new_node = malloc(sizeof(struct clientNode));
    new_node->client = *data; //must copy data here! shallow copy works
    new_node->next = NULL;

    if(*root==NULL){
        *root = new_node;
    }
    else{
        new_node->next = *root;
        *root = new_node;
    }
}


void printClient(struct client * client){
    printf("------------------------------------\n");
    printf(" Client ID: %s\n", client->clientID);
    printf("  Password: %s\n", client->password);
}

void printClientList(struct clientNode * root){
    struct clientNode * curr_node = root;
    while(curr_node != NULL){
        printClient(&curr_node->client);
        curr_node = curr_node->next;
    }
}


/*
int main(){

    struct client * client;
    char * clientID = "xunathan";
    client = find_client(clientID, CLIENT_DB);

    printf("%s\n", client->clientID);
    printf("%s\n", client->password);

    return 0;
}
*/
