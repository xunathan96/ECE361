//
//  server.c
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

    int status, socket_fd, new_socket_fd;
    struct addrinfo hints;
    struct addrinfo *res, *p;
    struct sockaddr_storage their_addr;     //connector's addr info

    //message variables
    char message[MAXDATASIZE], *response;
    int bytes_recv;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    //Connection based byte streams (TCP)
    hints.ai_protocol = IPPROTO_TCP;    //TCP
    hints.ai_flags = AI_PASSIVE;        //fill in my IP address

    status = getaddrinfo(NULL, MYPORT, &hints, &res);

    //CREATE AND BIND SOCKET TO PORT
    socket_fd = socket(res->ai_family,
                       res->ai_socktype,
                       res->ai_protocol);
    bind(socket_fd,
         res->ai_addr,
         res->ai_addrlen);

    freeaddrinfo(res);  //free the linked list

    listen(socket_fd, BACKLOG);
    printf("server: waiting for connections...\n");

    //LOOP THAT ACCEPTS MULTIPLE CONNECTION REQUESTS
    while (1) {

        //ACCEPT A REQUEST/CONNECTION + ESTABLISH A NEW SOCKET CONNECTION (TRANSFER CALL)
        socklen_t sin_size = sizeof their_addr;
        new_socket_fd = accept(socket_fd,
                              (struct sockaddr *)&their_addr,
                              &sin_size);

        //FORK TO LET CHILD PROCESS HANDLE COMMUNICATIONS ON THE NEW SOCKET
        if(!fork()){
            //child process doesn't need to work with the old socket
            close(socket_fd);

            //RECIEVE MESSAGE
            bytes_recv = recv(new_socket_fd,
                              message,
                              MAXDATASIZE-1,
                              0);
            message[bytes_recv] = '\0';

            printf("Message Recieved\n");
            printf("Client Message: %s\n", message);
            printf("Bytes Recieved: %d\n", bytes_recv);

            //SEND MESSAGE
            response = "2) Hello this is the server!";
            send(new_socket_fd,
                 response,
                 strlen(response),
                 0);

            //EXIT CHILD PROCESS
            exit(0);
        }

        //parent process doesn't need to work with the new socket... it goes back to accepting connection requests
        close(new_socket_fd);
    }

    return 0;
}
