#include "eventHandler.h"

int eventHandler(struct message * message, char * response, int socket_fd, struct ConnectionNode ** connectionList, struct clientNode ** clientList){

    //CLIENT AUTHENTICATION
    int isConnectedUser = searchClientList(*clientList, (char*)message->source);
    if(!isConnectedUser && message->type!=LOGIN){
        char * msg = "you must first log in...";
        sprintf(response, "%d:%lu:%s:%s", LO_NACK, strlen(msg), "server", msg);
        return SEND_NACK;
    }

    //DEAL WITH CLIENT REQUESTS
    switch (message->type) {
        case LOGIN: {
			
            char data[MAX_DATA];
            strcpy(data, (char*)message->data);

            char * clientID;
            char * password;
            char * s = " ";
            clientID = strtok(data, s);
            password = strtok(NULL, s);


            struct client * user;
            user = find_client(clientID , CLIENT_DB);
			//TODO check if clientID is already in clientList

			struct clientNode * curr_node = *clientList;
			while(curr_node != NULL){
					//printf("%d %d\n", curr_node->client.sockfd, socket_fd);
				if (curr_node->client.sockfd == socket_fd){
                    char * msg = "client is already logged in...logging out";
                    sprintf(response, "%d:%lu:%s:%s", LO_NACK, strlen(msg), "server", msg);
                    return LOGIN_FAIL;
				}
				curr_node = curr_node->next;
			}



            if(user==NULL){
                char * msg = "user not found...";
                sprintf(response, "%d:%lu:%s:%s", LO_NACK, strlen(msg), "server", msg);
                return LOGIN_FAIL;
            }
            else if(strcmp(user->password, password) == 0){
                //ADD CLIENT TO LIST OF LOGGED IN CLIENTS
                int isLoggedIn = searchClientList(*clientList, user->clientID);
                if(!isLoggedIn){
					user->sockfd = socket_fd;
                    addToClientList(clientList, user);
                }
                else{
                    //CLIENT IS ALREADY LOGGED IN
                    char * msg = "user is already logged in...";
                    sprintf(response, "%d:%lu:%s:%s", LO_NACK, strlen(msg), "server", msg);
                    return LOGIN_FAIL;
                }
                //SEND LOGIN ACK
                char * msg = "login successful...";
                sprintf(response, "%d:%lu:%s:%s", LO_ACK, strlen(msg), "server", msg);
                return SEND_ACK;
            }
            else{
                char * msg = "incorrect password...";
                sprintf(response, "%d:%lu:%s:%s", LO_NACK, strlen(msg), "server", msg);
                return LOGIN_FAIL;
            }
        }
        case NEW_SESS: {
            char session[MAX_DATA];
            char clientID[MAX_NAME];
            strcpy(session, (char*)message->data);
            strcpy(clientID, (char*)message->source);

            //SESSION ID ALREADY TAKEN
            struct ConnectionNode * sessionList;
            sessionList = findSessionConnections(*connectionList, session);
            if(sessionList!=NULL){
                char * msg = "session already exists...";
                sprintf(response, "%d:%lu:%s:%s", NS_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }

            //CLIENT ALREADY IN A SESSION/CONNECTION
            struct ConnectionNode * clientConnections;
            clientConnections = findClientConnections(*connectionList, clientID);
            if(clientConnections!=NULL){
                char * msg = "you must first leave your current session...";
                sprintf(response, "%d:%lu:%s:%s", NS_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }

            //GET ADDR INFO OF CONNECTED HOST
            struct sockaddr_in their_addr;
            socklen_t len = sizeof their_addr;
            getpeername(socket_fd, (struct sockaddr *)&their_addr, &len);

            //CREATE AND ADD CONNECTION TO CONNECTION LIST
            struct Connection new_connection;
            createConnection(&new_connection, their_addr, clientID, session, socket_fd);
            addConnection(connectionList, &new_connection);

            char * msg = "created and joined new session...";
            sprintf(response, "%d:%lu:%s:%s", NS_ACK, strlen(msg), "server", msg);
            return SEND_ACK;
        }
        case LEAVE_SESS: {
            int isDeleted;
            isDeleted = deleteConnectionAtSocket(connectionList, socket_fd);
            if(isDeleted==0){
                char * msg = "you can not leave a session you are not in...";
                sprintf(response, "%d:%lu:%s:%s", LS_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }
            else if(isDeleted==1){
                char * msg = "you have left the session...";
                sprintf(response, "%d:%lu:%s:%s", LS_ACK, strlen(msg), "server", msg);
                return SEND_ACK;
            }
        }
        case JOIN: {
            char session[MAX_DATA];
            char clientID[MAX_NAME];
            strcpy(session, (char*)message->data);
            strcpy(clientID, (char*)message->source);

            //SESSION DOESNT EXIST
            struct ConnectionNode * sessionList;
            sessionList = findSessionConnections(*connectionList, session);
            if(sessionList==NULL){
                char * msg = "can not find session...";
                sprintf(response, "%d:%lu:%s:%s", JN_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }

            //CLIENT ALREADY IN A SESSION/CONNECTION
            struct ConnectionNode * clientConnections;
            clientConnections = findClientConnections(*connectionList, clientID);
            if(clientConnections!=NULL){
                char * msg = "you must first leave your current session...";
                sprintf(response, "%d:%lu:%s:%s", JN_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }

            //GET ADDR INFO OF CONNECTED HOST
            struct sockaddr_in their_addr;
            socklen_t len = sizeof their_addr;
            getpeername(socket_fd, (struct sockaddr *)&their_addr, &len);

            //CREATE AND ADD CONNECTION TO CONNECTION LIST
            struct Connection new_connection;
            createConnection(&new_connection, their_addr, clientID, session, socket_fd);
            addConnection(connectionList, &new_connection);

            char * msg = "you have joined the session...";
            sprintf(response, "%d:%lu:%s:%s", JN_ACK, strlen(msg), "server", msg);
            return SEND_ACK;
        }
        case MESSAGE: {
            char send_message[MAX_DATA];
            char clientID[MAX_NAME];
            strcpy(send_message, (char*)message->data);
            strcpy(clientID, (char*)message->source);

            //IF CLIENT IS NOT IN ANY SESSION SEND NACK
            struct ConnectionNode * clientConnections;
            clientConnections = findClientConnections(*connectionList, clientID);
            if(clientConnections==NULL){
                char * msg = "you are not connected to any session...";
                sprintf(response, "%d:%lu:%s:%s", MESSAGE_NACK, strlen(msg), "server", msg);
                return SEND_NACK;
            }

            sprintf(response, "%d:%lu:%s:%s", MESSAGE, strlen(send_message), clientID, send_message);
            return MULTICAST;
        }
        case QUERY: {
            char serializedList[MAX_DATA];
            serializeConnectionList(*connectionList, serializedList);
			if (strlen(serializedList)!=0){
            	sprintf(response, "%d:%lu:%s:%s", QU_ACK, strlen(serializedList), "server", serializedList);
			}
			else{
				sprintf(response, "%d:%lu:%s:%s", QU_ACK, strlen("no active sessions..."), "server", "no active sessions...");
			}
            return SEND_ACK;
        }
        case EXIT: {
            char clientID[MAX_NAME];
            strcpy(clientID, (char*)message->source);

            //REMOVE CLIENT FROM LIST OF LOGGED IN USERS
            removeFromClientList(clientList, clientID);
            return CLOSE;
        }
        default:{
            char * msg = "unknown command...";
            sprintf(response, "%d:%lu:%s:%s", UNKNOWN_COMMAND, strlen(msg), "server", msg);
            return SEND_NACK;
        }
    }
    return 0;
}



int unicast(int sendto_socket, char * message){
    return send(sendto_socket,
                message,
                strlen(message),
                0);
}


int multicast(struct ConnectionNode * sessionList, char * message, int except_socket){
    if(sessionList==NULL){return 0;}
    struct ConnectionNode * curr_node = sessionList;
    while(curr_node!=NULL){
        int sock_fd = curr_node->connection.socket_fd;
        if(except_socket != sock_fd){
            //SEND MESSAGE TO DESTINATION CLIENTS
            send(sock_fd,
                 message,
                 strlen(message),
                 0);
         }
         else{
             //SEND ACK TO SOURCE CLIENT
             char response[MAX_DATA];
             char * msg = "message sent...";
             sprintf(response, "%d:%lu:%s:%s", MESSAGE_ACK, strlen(msg), "server", msg);
             send(sock_fd,
                  response,
                  strlen(response),
                  0);
         }
        curr_node = curr_node->next;
    }
    return 1;
}


void serializeConnectionList(struct ConnectionNode * connectionList, char * serializedList){
    char buffer[MAX_DATA];
    serializedList[0] = '\0';

    struct ConnectionNode * curr_node = connectionList;
    while(curr_node!=NULL){

        sprintf(buffer, "%s:%s", curr_node->connection.clientID, curr_node->connection.sessionID);
        strcat(serializedList, buffer);
        buffer[0] = '\0';   //CLEAR BUFFER

        curr_node = curr_node->next;
        if(curr_node!=NULL){
            strcat(serializedList, "\n");
        }
    }
}


//
