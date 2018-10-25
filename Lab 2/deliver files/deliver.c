//
//  deliver.c
//
//
//  Created by Nathaniel Xu on 2018-09-23.
//
//

#include "packet.h"

#define SERVERPORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1100


int main(int argc, char *argv[]){

    if (argc!=4) {
        printf("invalid number of arguments\n");
        return 0;
    }
    if(strcmp(argv[1], "deliver")!=0){
        printf("invalid command: %s\n", argv[1]);
        return 0;
    }
    char *ip_addr = argv[2];
    int port_num = atoi(argv[3]);
    if (port_num==0 && argv[3][0]!='0') {
        printf("invalid port number: %s\n", argv[3]);
        return 0;
    }
    char * theirport = argv[3];


    printf("please enter a filename to transfer in the following format: ftp <filename>\n");
    char input[50], filename[50];
    scanf("%s %s", input, filename);

    if(strcmp(input, "ftp")!=0){
        printf("invalid command: %s\n", input);
        return 0;
    }

    if(access(filename, F_OK)!=0){
        printf("file not found\n");
        return 0;
    }

    //--------------------------------------------------------------------------


    int status, socket_fd;
    struct addrinfo hints;
    struct addrinfo *server_addr;
    struct sockaddr_storage server_sockaddr;     //connector's addr info

    //message variables
    char message[MAXDATASIZE], *response;
    int bytes_recv;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;     //Connectionless data transfer of fixed max length datagrams
    hints.ai_protocol = IPPROTO_UDP;    //UDP

    status = getaddrinfo(ip_addr, theirport, &hints, &server_addr);

    //GET A SOCKET FROM SERVER AT SPECIFIED PORT
    socket_fd = socket(server_addr->ai_family,
                       server_addr->ai_socktype,
                       server_addr->ai_protocol);

    //SEND DATAGRAM
    response = "ftp";
    clock_t start, end;
    start = clock();
    sendto(socket_fd,
           response,
           strlen(response),
           0,
           server_addr->ai_addr,
           server_addr->ai_addrlen);


    //RECIEVE DATAGRAM
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    bytes_recv = recvfrom(socket_fd,
                          message,
                          MAXDATASIZE-1 ,
                          0,
                          (struct sockaddr *)&server_sockaddr,
                          &addr_len);
    end = clock();
    message[bytes_recv] = '\0';
    printf("RTT: %f\n", ((double) (end - start) / CLOCKS_PER_SEC));

    if(strcmp(message, "yes")==0){
        printf("A file transfer can start.\n");
    }
    else{
        return 0;
    }

    //--------------------------------------------------------------------------
    //START SENDING PACKETS
    struct packet * root_packet = fragment_file(filename);
    struct packet * curr_packet = root_packet;
    int len;

    while(curr_packet!=NULL){
        char * condensedPacket = condense_packet(curr_packet, &len);
        //printPacket(curr_packet);

        //SEND PACKET
        bytes_recv = sendto(socket_fd,
               condensedPacket,
               len,
               0,
               server_addr->ai_addr,
               server_addr->ai_addrlen);

        //RECIEVE PACKET
        bytes_recv = recvfrom(socket_fd,
                              message,
                              MAXDATASIZE-1,
                              0,
                              (struct sockaddr *)&server_sockaddr,
                              &addr_len);
        message[bytes_recv] = '\0';

        //IF NOT SEGMENT NOT ACK THEN RESEND SEGMENT
        if(strcmp(message, "ACK")!=0){
            continue;
        }

        printf("Packet #%d Sent.\n", curr_packet->frag_no);

        curr_packet = curr_packet->next_packet;
        free(condensedPacket);
    }

    freePackets(root_packet);
    freeaddrinfo(server_addr);  //free the linked list
    close(socket_fd);
    return 0;
}
