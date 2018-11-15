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
#include <errno.h>

#define MYPORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100


int main(int argc, char *argv[]){
    
    if (argc!=3) {
        printf("invalid number of arguments\n");
        return 0;
    }
    if(strcmp(argv[1], "server")!=0){
        printf("invalid command: %s\n", argv[1]);
        return 0;
    }
    int port_num = atoi(argv[2]);
    if (port_num==0 && argv[2][0]!='0') {
        printf("invalid port number: %s\n", argv[2]);
        return 0;
    }
    char * myport = argv[2];
    
    //--------------------------------------------------------------------------

    int status, socket_fd;
    struct addrinfo hints;
    struct addrinfo *server_addr;
    struct sockaddr_storage client_sockaddr;     //why can this not be a pointer??

    //message variables
    char message[MAXDATASIZE], *response;
    int bytes_recv;
    
    memset(&hints, 0, sizeof hints);
    
    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;     //Connectionless data transfer of fixed maxlength datagrams
    hints.ai_protocol = IPPROTO_UDP;    //UDP
    hints.ai_flags = AI_PASSIVE;        //fill in my IP address
    
    getaddrinfo(NULL, myport, &hints, &server_addr);

    //CREATE AND BIND SOCKET TO PORT
    socket_fd = socket(server_addr->ai_family,
                       server_addr->ai_socktype,
                       server_addr->ai_protocol);
    bind(socket_fd,
         server_addr->ai_addr,
         server_addr->ai_addrlen);

    
    //NO NEED TO LISTEN AND ACCEPT!
    printf("Server: waiting to recvfrom...\n");
    
    
    //RECIEVE DATAGRAM
    socklen_t addr_len = sizeof(struct sockaddr_storage);   //or sizeof(client_sockaddr)
    bytes_recv = recvfrom(socket_fd,
                          message,
                          MAXDATASIZE-1 ,
                          0,
                          (struct sockaddr *)&client_sockaddr,      //why can client_sockaddr not be a *?
                          &addr_len);
    
    message[bytes_recv] = '\0';
    
    //printf("Message Recieved\n");
    //printf("Client Message: %s\n", message);
    //printf("Bytes Recieved: %d\n", bytes_recv);
    
    //BUISNESS LOGIC
    if(strcmp(message, "ftp")==0) {
        response = "yes";
    }
    else{
        response = "no";
    }

    
    //SEND DATAGRAM
    status = sendto(socket_fd,
                    response,
                    strlen(response),
                    0,
                    (struct sockaddr *)&client_sockaddr,
                    addr_len);
    
    //if(status == -1) {
    //    printf("Error sending: %i %s\n",errno, strerror(errno));
    //}

    freeaddrinfo(server_addr);  //free the linked list
    close(socket_fd);
    return 0;
}









