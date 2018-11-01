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
#define TRUE 1
#define FALSE 0
#define ALPHA 0.125
#define BETA 0.25


void setTimeout(struct timeval * timeout, double RTO){
    timeout->tv_sec = RTO/1;
    timeout->tv_usec = (RTO - timeout->tv_sec)*1000000;
}


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

    // RTT calculated
    double RTT = (double) (end - start) / CLOCKS_PER_SEC;
    printf("RTT: %f\n", RTT);

    if(strcmp(message, "yes")==0){
        printf("A file transfer can start.\n");
    }
    else{
        return 0;
    }

    //--------------------------------------------------------------------------

    int isRetransmission = FALSE;

    // Set initial timeout values based on RFC6298
    struct timeval timeout;
    double SRTT, RTTVAR, RTO;
    SRTT = RTT;
    RTTVAR = RTT/2;
    RTO = SRTT + 4*RTTVAR;

    // Set file descriptor set for incoming segments
    fd_set readfds;
    FD_ZERO(&readfds);

    //START SENDING PACKETS
    struct packet * root_packet = fragment_file(filename);
    struct packet * curr_packet = root_packet;
    int len;

    while(curr_packet!=NULL){
        char * condensedPacket = condense_packet(curr_packet, &len);

        //SEND PACKET
        start = clock();
        bytes_recv = sendto(socket_fd,
                            condensedPacket,
                            len,
                            0,
                            server_addr->ai_addr,
                            server_addr->ai_addrlen);

        //MONITOR socket_fd AND SET TIMEOUT
        setTimeout(&timeout, RTO);
        FD_SET(socket_fd, &readfds);
        select(socket_fd+1, &readfds, NULL, NULL, &timeout);

        //TIMEOUT
        if(!FD_ISSET(socket_fd, &readfds)){
            // RETRANSMIT PACKET
	    printf("TIMEOUT\n");
            isRetransmission = TRUE;    //Mark next set of ACKs is for retrasmitted data
            RTO = RTO*2;                //Back off the timer
            free(condensedPacket);
            continue;
        }

        //RECIEVE PACKET
        bytes_recv = recvfrom(socket_fd,
                              message,
                              MAXDATASIZE-1,
                              0,
                              (struct sockaddr *)&server_sockaddr,
                              &addr_len);
        end = clock();
        message[bytes_recv] = '\0';

        //IF SEGMENT IS NOT ACKed THEN RETRANSMIT SEGMENT
        if(strcmp(message, "ACK")!=0){
            isRetransmission = TRUE;    //Mark next set of ACKs is for retrasmitted data
            free(condensedPacket);
            continue;
        }


        printf("Packet #%d Sent and Acknowledged.\n", curr_packet->frag_no);

        //CHECK IF TRANSFER ROUND WAS FOR RETRANSMISSION OF LOST PACKET
        //Do not calculate timeout value based on a retransmission round
        if(isRetransmission){
            isRetransmission = FALSE;
        }
        else{
            //CALCULATE NEW TIMEOUT VALUE
            RTT = (double) (end - start) / CLOCKS_PER_SEC;
            RTTVAR = (1-BETA)*RTTVAR + BETA*fabs(SRTT - RTT);
            SRTT = (1-ALPHA)*SRTT + ALPHA*RTT;
            RTO = SRTT + 4*RTTVAR;
            if(RTO<1){
                RTO = 1;
            }
        }

        curr_packet = curr_packet->next_packet;
        free(condensedPacket);
    }

    //free the linked lists
    freePackets(root_packet);
    freeaddrinfo(server_addr);
    close(socket_fd);
    return 0;
}
