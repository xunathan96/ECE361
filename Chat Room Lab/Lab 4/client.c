#include "initialization.h"
#include "message.h"
#include "eventHandler.h"
#define STDIN 0
int main(){

	
    int status, socket_fd;
    struct addrinfo hints;
    struct addrinfo *res, *p;

    //message variables
  
    int bytes_recv;
    char input[MAX_DATA];
	char server_input[MAX_DATA];
	char myusername[100];
	char message[MAX_DATA];
	fd_set readfds;
	FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds); //0 for STDIN
	int fdmax = STDIN;
	socket_fd = -1; //set initial to -1 so we aren't reading from unitialized socket
	int loginflag = 0;
    while(1){
		fprintf(stdout, "\n> ");
		fflush(stdout);
		memset(input, 0, sizeof(input));
		memset(server_input, 0, sizeof(server_input));
		memset(message, 0, sizeof(message));
		FD_ZERO(&readfds); //reset fd set with updated socket_fd
		FD_SET(STDIN, &readfds); 
		if (socket_fd != -1){
			FD_SET(socket_fd,&readfds); 
			fdmax = socket_fd;
		}
		else{
			fdmax = STDIN;
		}

		if(select(fdmax+1,&readfds,NULL,NULL,NULL)<0){ //blocks on here till something to be read from a FD
        	fprintf(stdout, "select() error\n");
        	exit(0);
    	}   
		
        		
	
		if(socket_fd != -1 && FD_ISSET(socket_fd,&readfds)){
		    //RECIEVE MESSAGE
		    bytes_recv = recv(socket_fd, message, MAX_DATA-1, 0);

			//printf("|%s|\n", message);
			message[bytes_recv] = '\0';
			struct message * response;
			response =  deserialize_message(message);
			//printMessage(response);
			  // printf("Message Recieved-----------------\n");
			   //printf("Server Message: %s\n", message);
			   // printf("Bytes Recieved: %d\n", bytes_recv);
			//printf("response|%d|\n", response->type);
			switch (response->type){
				case LO_NACK:
					printf("%s: %s\n", response->source, response->data);
					socket_fd = -1;
					loginflag = 0;
					break;
				case MESSAGE:
				case MESSAGE_NACK:
				case LO_ACK:
				case JN_ACK:
				case LS_ACK: 	
				case NS_ACK:
				case JN_NACK:
				case LS_NACK:
				case NS_NACK:
					printf("%s: %s\n", response->source, response->data);
					break;
				case QU_ACK:
					//TODO parse session list
					printf("%s\n", response->data);
					break;
				
			}//end switch(response->type)
				
		}
		
		if(FD_ISSET(STDIN,&readfds)) {
			fgets(input,MAX_DATA,stdin);
			input[strlen(input)-1]='\0';
			//parse input
			if (input[0] == '/'){
				char * s = " ";
				char * command;
				char * data;
		        command = strtok(input, s);
				

				if (strcmp(command, "/login") == 0){
					char* username;
					char* password;
					char* server_addr;
					char* server_port;
					username = strtok(NULL, s);
					password = strtok(NULL, s);
					server_addr = strtok(NULL, s);
					server_port = strtok(NULL, s);
					if (!(username == NULL || password == NULL || server_addr == NULL || server_port == NULL)){
						//printf("inputs|%s|%s|%s|%s|\n", username, password, server_addr, server_port);
						strcpy(myusername, username);
						//printf("username|%s|\n", myusername);
						if (loginflag == 0){ //only create new socket if not already logged in
							memset(&hints, 0, sizeof hints);
							hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
							hints.ai_socktype = SOCK_STREAM;    //Connection based byte streams (TCP)
							hints.ai_protocol = IPPROTO_TCP;    //TCP
							hints.ai_flags = AI_PASSIVE;        //fill in my IP address

							status = getaddrinfo(NULL, server_port, &hints, &res);


							//CREATE A SOCKET AND CONNECT TO ...
							socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
							if(connect(socket_fd, res->ai_addr, res->ai_addrlen)!= 0){ // did not connect
								socket_fd = -1;
								loginflag = 0;
							}
							else{
								loginflag = 1;
							}
							freeaddrinfo(res);  //free linked list
							
						}
						sprintf(server_input, "10:%d:test:%s %s", strlen(username)+strlen(password) + 1, username, password);
					}
					else{
						printf("error: usage /login <username> <password> <server_addr> <server_port>\n");
					}
				}

				else if (strcmp(command, "/logout") == 0){
					sprintf(server_input, "20:%d:%s:%s", strlen("test"), myusername, "test");
					//close(socket_fd);
					send(socket_fd, server_input, strlen(server_input), 0);
					socket_fd = -1;
					loginflag = 0;
					memset(myusername, 0, sizeof(myusername));
				}
	
				else if (strcmp(command, "/joinsession") == 0){
					char * session_name;
					session_name = strtok(NULL, s);
					sprintf(server_input, "30:%d:%s:%s", strlen(session_name), myusername, session_name);	
				}

				else if (strcmp(command, "/leavesession") == 0){
					sprintf(server_input, "40:%d:%s:%s", 0, myusername, "test");	
				}

				else if (strcmp(command, "/createsession") == 0){
					char * session_name;
					session_name = strtok(NULL, s);
					if (session_name != NULL){
						sprintf(server_input, "50:%d:%s:%s", strlen(session_name), myusername, session_name);
					}
					else{
						printf("error: usage /createsession <session_name>\n");
					}
				}

				else if (strcmp(command, "/list") == 0){
					sprintf(server_input, "70:%d:%s:%s", strlen("test"), myusername, "test");
				}

				else if (strcmp(command, "/quit") == 0){
					sprintf(server_input, "20:%d:%s:%s", strlen("test"), myusername, "test");
					break;
				}
				else{
					printf("unknown command:%s\n", command);
				}
			
			}//end if input[0] == '/'
		
	
			else{ //else input[0] != '/' , send a message
				sprintf(server_input, "60:%d:%s:%s", strlen(input), myusername, input);
			}

			//printf("sending|%s|to server\n", server_input);
        	send(socket_fd, server_input, strlen(server_input), 0);

		}//end if (FD_ISSET(STDIN,&readfds))










    }
	printf("exiting client\n");
    close(socket_fd);

    return 0;
}
