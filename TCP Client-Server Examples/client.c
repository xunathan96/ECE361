//
//  client.c
//
//
//  Created by Nathaniel Xu on 2018-09-23.
//
//

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>     //includes sockaddr
#include <netdb.h>          //includes addrinfo
#include <arpa/inet.h>      //includes inet_ntop etc...
#include <unistd.h>         //includes close()
#include <stdlib.h>         //includes exit(1)

#define MYPORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100


int main(){

    int status, socket_fd;
    struct addrinfo hints;
    struct addrinfo *res, *p;

    //message variables
    char message[MAXDATASIZE], *response;
    int bytes_recv;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    //Connection based byte streams (TCP)
    hints.ai_protocol = IPPROTO_TCP;    //TCP
    hints.ai_flags = AI_PASSIVE;        //fill in my IP address

    status = getaddrinfo(NULL, MYPORT, &hints, &res);

    //CREATE A SOCKET AND CONNECT TO ...
    socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(socket_fd, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);  //free linked list


    //SEND MESSAGE
    response = "1) Hello this is the client!";
    send(socket_fd, response, strlen(response), 0);


    //RECIEVE MESSAGE
    bytes_recv = recv(socket_fd, message, MAXDATASIZE-1, 0);
    message[bytes_recv] = '\0';

    printf("Message Recieved\n");
    printf("Server Message: %s\n", message);
    printf("Bytes Recieved: %d\n", bytes_recv);


    close(socket_fd);

    return 0;
}
