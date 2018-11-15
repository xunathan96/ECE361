#include "initialization.h"
#include "message.h"
#include "eventHandler.h"

//GLOBAL VARS/STRUCTS THAT ACTS AS DATABASES
struct ConnectionNode * connectionList = NULL;
struct clientNode * clientList = NULL;

int main(int argc, char *argv[]){
	if (argv[1] == NULL){
		printf("usage: ./server <port>\n");
		return -1;
	}

    int status, socket_fd;
    struct addrinfo hints;
    struct addrinfo *res, *p;
    struct sockaddr_storage their_addr;     //connector's addr info

    // Set file descriptor set for incoming segments
    fd_set master;
    fd_set read_fds;
    int fdmax;              //largest socket fd

    int listener;           //socket fd for incoming connetions
    int new_socket_fd;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    //message variables
    char message[MAX_DATA];
    char response[MAX_DATA];
    int bytes_recv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    //Connection based byte streams (TCP)
    hints.ai_protocol = IPPROTO_TCP;    //TCP
    hints.ai_flags = AI_PASSIVE;        //fill in my IP address
	//TODO server port as command line arg DONE
    status = getaddrinfo(NULL, argv[1], &hints, &res);

    //CREATE AND BIND SOCKET TO PORT
    listener = socket(res->ai_family,
                      res->ai_socktype,
                      res->ai_protocol);
    bind(listener,
         res->ai_addr,
         res->ai_addrlen);
    freeaddrinfo(res);  //free the linked list

    listen(listener, BACKLOG);
    printf("server: waiting for connections...\n");

    //ADD LISTENER SOCKET FD TO MASTER SET
    FD_SET(listener, &master);
    fdmax = listener;   //keep track of largest socket fd


    //LOOP THAT ACCEPTS MULTIPLE CONNECTION REQUESTS
    while (TRUE) {
        //Copy the master fd set to the read fd set
        read_fds = master;
        select(fdmax+1, &read_fds, NULL, NULL, NULL);

        //LOOP THROUGH ALL SOCKET FDS to LOOK FOR DATA THAT CAN BE READ
        for(int i=0; i<fdmax+1; i++){
            //SOME SOCKET IS READY TO BE READ FROM
            if(FD_ISSET(i, &read_fds)){
                //NEW CONNECTION AT LISTENER SOCKET
                if(i==listener){
                    //ACCEPT A REQUEST/CONNECTION +
                    //ESTABLISH A NEW SOCKET CONNECTION (3 way handshake)
                    socklen_t sin_size = sizeof their_addr;
                    new_socket_fd = accept(listener,
                                          (struct sockaddr *)&their_addr,
                                          &sin_size);

                    //ADD NEW SOCKET FD TO MASTER FD SET and
                    //keep track of max fd
                    FD_SET(new_socket_fd, &master);
                    if(new_socket_fd > fdmax){
                        fdmax = new_socket_fd;
                    }
                }
                //RECIEVING DATA FROM A CONNECTED CLIENT
                else{
                    bytes_recv = recv(i,
                                      message,
                                      MAX_DATA-1,
                                      0);
                    message[bytes_recv] = '\0';
					printf("recieved %s\n");
                    //DEAL WITH RECIEVED DATA
                    if(bytes_recv>0){
                        //PARSE MESSAGE
                        struct message * recv_message;
                        recv_message = deserialize_message(message);
                        printMessage(recv_message);

                        char clientID[MAX_NAME];
                        strcpy(clientID, (char*)recv_message->source);

                        int eventType;
                        eventType = eventHandler(recv_message, response, i, &connectionList, &clientList);
                        printConnectionList(connectionList);

                        if(eventType == SEND_ACK || eventType == SEND_NACK || eventType == LOGIN_FAIL){
                            unicast(i, response);
                        }
                        if(eventType == MULTICAST){
                            //GET ALL CLIENT CONNECTIONS
                            struct ConnectionNode * clientConnections;
                            clientConnections = findClientConnections(connectionList, clientID);

                            //GET CLIENT'S (first.. for now) SESSION ID
                            char sessionID[MAX_SESSION_ID];
                            strcpy(sessionID, clientConnections->connection.sessionID);

                            //GET ALL CONNECTIONS TO THAT SESSION ID
                            struct ConnectionNode * sessionList;
                            sessionList = findSessionConnections(connectionList, sessionID);

                            //MULTICAST TO ALL CONNECTIONS IN SPECIFIED SESSION
                            multicast(sessionList, response, i);
                        }
                        if(eventType == CLOSE  || eventType == LOGIN_FAIL ){
                            printf("closing connection at socket: %d\n", i);
							removeFromClientListBySock(&clientList , i); // remove client on this socket from clientlist
							deleteConnectionAtSocket(&connectionList, i);
                            close(i);
                            FD_CLR(i, &master);
                        }
                    }
                    //CONNECTION CLOSED
                    else{
						//TODO remove client from sessionlist/clientlist
                        printf("serverinfo: socket %d hung up\n", i);
						removeFromClientListBySock(&clientList , i); //remove client on this socket from clientlist
						deleteConnectionAtSocket(&connectionList, i);
                        //CLOSE SOCKET AND REMOVE FD FROM MASTER
                        close(i);
                        FD_CLR(i, &master);
                    }
                }
            }
        }
    }

    return 0;
}
