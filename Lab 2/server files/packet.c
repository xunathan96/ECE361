//
//  server.c
//
//
//  Created by Nathaniel Xu on 2018-09-23.
//
//

#include "packet.h"


void printPacket(struct packet * pack){
    printf("total_frag: %d\n", pack->total_frag);
    printf("frag_no: %d\n", pack->frag_no);
    printf("size: %d\n", pack->size);
    printf("filename: %s\n", pack->filename);
    printf("filedata: %s\n", pack->filedata);
}
void printPacketList(struct packet * p){
    while(p!=NULL){
        printPacket(p);
        printf("--------------------------------------------------\n");
        p = p->next_packet;
    }
}

struct packet * fragment_file(char * filename){
    FILE * fp;
    int num_bytes, total_frag, size, frag_no;
    char data[FRAGSIZE];
    struct packet * root_packet, * prev_packet;

    fp = fopen(filename, "rb");     // add b for binary files

    // DETERMINE TOTAL NUMBER OF BYTES IN FILE
    fseek(fp, 0, SEEK_END);
    num_bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // DETERMINE TOTAL NUMBER OF FRAGMENTS
    total_frag = (num_bytes/FRAGSIZE) + 1;

    // CREATE THE LINKED LIST OF PACKETS
    for (frag_no=1; frag_no<=total_frag; frag_no++) {
        struct packet * new_packet = malloc(sizeof(struct packet));
        if(frag_no==1){
            root_packet = new_packet;
        }
        else{
            prev_packet->next_packet = new_packet;
        }

        size = fread(data, 1, FRAGSIZE, fp);    //warning! data has no string terminator /0

        new_packet->total_frag = total_frag;
        new_packet->frag_no = frag_no;
        new_packet->size = size;
        new_packet->filename = filename;
        memcpy(new_packet->filedata, data, size);
        // use MEMCPY and not STRCPY b/c data might not be a string!!
        // Data may not be null terminated. Could be binaries
        new_packet->next_packet = NULL;

        prev_packet = new_packet;
    }

    fclose(fp);
    return root_packet;
};

void freePackets(struct packet * root){
    struct packet * curr_packet = root;
    struct packet * next_packet;
    while (curr_packet!=NULL) {
        next_packet = curr_packet->next_packet;
        free(curr_packet);
        curr_packet = next_packet;
    }
}

char * condense_packet(struct packet * pack, int * len){

    int s1 = snprintf(NULL, 0, "%d", pack->total_frag);
    int s2 = snprintf(NULL, 0, "%d", pack->frag_no);
    int s3 = snprintf(NULL, 0, "%d", pack->size);
    int s4 = strlen(pack->filename);
    int s5 = pack->size;
    int total_size = s1+s2+s3+s4+s5;

    char * condensedPacket = malloc((total_size+4)*sizeof(char));

    int header_offset = sprintf(condensedPacket, "%d:%d:%d:%s:",
                                pack->total_frag,
                                pack->frag_no,
                                pack->size,
                                pack->filename);

    memcpy(&condensedPacket[header_offset], pack->filedata, pack->size);

    *len = header_offset + pack->size;
    return condensedPacket;
}

struct packet * extract_packet(char * packet_str){
    struct packet * extractedPacket;
    char *total_frag_str, *frag_no_str, *size_str, *filename, *filedata;
    int total_frag, frag_no, size;

    total_frag_str = strtok(packet_str, ":");
    frag_no_str = strtok(NULL, ":");
    size_str = strtok(NULL, ":");
    filename = strtok(NULL, ":");

    total_frag = atoi(total_frag_str);
    frag_no = atoi(frag_no_str);
    size = atoi(size_str);

    //GET THE DATA WITHOUT USING STR FUNCTIONS
    int udp_header_size = strlen(total_frag_str) +
                          strlen(frag_no_str) +
                          strlen(size_str) +
                          strlen(filename) +
                          4;

    filedata = malloc(size*sizeof(char));
    memcpy(filedata, &packet_str[udp_header_size], size);


    extractedPacket = malloc(sizeof(struct packet));
    extractedPacket->total_frag = total_frag;
    extractedPacket->frag_no = frag_no;
    extractedPacket->size = size;
    extractedPacket->filename = filename;
    memcpy(extractedPacket->filedata, filedata, size);
    if(size < FRAGSIZE){
        extractedPacket->filedata[size] = '\0';
    }

    return extractedPacket;
}

/*
int main(){

    struct packet * root_packet;

    root_packet = fragment_file("file.txt");
    printPacket(root_packet);

    char * s = condense_packet(root_packet);
    //printf("%s\n", s);

    struct packet * new_packet = extract_packet(s);
    printPacket(new_packet);

    //printPacketList(root_packet);
    freePackets(root_packet);

    return 0;
}*/
