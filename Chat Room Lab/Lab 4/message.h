#ifndef MESSAGE_H
#define MESSAGE_H

#include "initialization.h"

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void printMessage(struct message * message);

//GO FROM type:size:source:data TO message structure
struct message * deserialize_message(char* message);

//GO FROM message structure TO type:size:source:data
char * serialize_message(struct message * message);

#endif
