#ifndef INITIALIZATION_H
#define INITIALIZATION_H

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

#define MYPORT "3490"
#define BACKLOG 10

#define TRUE 1
#define FALSE 0

#define MAX_NAME 1000
#define MAX_DATA 50000
#define MAX_PASSWORD 1000
#define MAX_SESSION_ID 1000
#define MAX_IPADDR 1000




#endif
