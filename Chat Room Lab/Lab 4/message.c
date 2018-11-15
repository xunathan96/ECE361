#include "message.h"

struct message * deserialize_message(char* message){
	//TODO handle unexpected amount of ':'
    char* command;
    char* size;
    char* source;
    char* data;
    char* s = ":";
    if(message==NULL){return NULL;}
    struct message * new_message = malloc(sizeof(struct message));

    command = strtok(message, s);
    size = strtok(NULL, s);
    source = strtok(NULL, s);
    data = strtok(NULL, "");
	if (command == NULL || size == NULL || source == NULL || data == NULL){return NULL;}

    new_message->type = atoi(command);
    new_message->size = atoi(size);
    strcpy((char*)new_message->source, source);
    strcpy((char*)new_message->data, data);

    return new_message;
}

char * serialize_message(struct message * message){
    int s1 = snprintf(NULL, 0, "%d", message->type);
    int s2 = snprintf(NULL, 0, "%d", message->size);
    int s3 = strlen((char*)message->source);
    int s4 = strlen((char*)message->data);
    int total_length = s1+s2+s3+s4;

    char * new_message = malloc((total_length+4)*sizeof(char));

    sprintf(new_message, "%d:%d:%s:%s",
            message->type,
            message->size,
            message->source,
            message->data);

    return new_message;
}

void printMessage(struct message * message){
    printf("Type: %d\n", message->type);
    printf("Size: %d\n", message->size);
    printf("Source: %s\n", message->source);
    printf("Data: %s\n", message->data);
}

/*
int main(){

    char message1[] = "login arg1 arg2";
    char message2[] = "LOGIN(int):sizeofdata(int):source(clientID):data";
    char message3[] = "12:131:source(clientID):data";

    struct message * message;
    message = deserialize_message(message3);
    printf("%d\n", message->type);
    printf("%d\n", message->size);
    printf("%s\n", message->source);
    printf("%s\n", message->data);

    char * str_message;
    str_message = serialize_message(message);
    printf("%s\n", str_message);


    return 0;
}
*/
