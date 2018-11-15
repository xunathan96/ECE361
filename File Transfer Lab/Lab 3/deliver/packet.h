#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>     //includes sockaddr
#include <netdb.h>          //includes addrinfo
#include <arpa/inet.h>      //includes inet_ntop etc...
#include <unistd.h>         //includes close()
#include <stdlib.h>         //includes exit(1)
#include <errno.h>
#include <time.h>
#include <math.h>

#define FRAGSIZE 1000

struct packet {
    unsigned int total_frag;        // Total number of fragments
    unsigned int frag_no;           // Fragment number of packet
    unsigned int size;              // Size of data in bytes
    char* filename;
    char filedata[FRAGSIZE];
    struct packet * next_packet;
};


struct packet * fragment_file(char * filename);

void freePackets(struct packet * root);

char * condense_packet(struct packet * pack, int * len);
struct packet * extract_packet(char * packet_str);

void printPacket(struct packet * pack);
void printPacketList(struct packet * p);





#endif
