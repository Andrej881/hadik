#include "comunication.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>      
#include <fcntl.h>

#define MAX_BUF 10000

typedef struct ClientGameInfo
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    GameInfo game;
} ClientGameInfo;


void NewGame(ClientGameInfo* info);
int JoinGame(ClientGameInfo* info, int port, const char* ip);
void Run(ClientGameInfo* info);