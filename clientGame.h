#include "comunication.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>      
#include <fcntl.h>
#include <pthread.h>

#define MAX_BUF 10000

typedef struct ClientGameInfo
{
    struct termios original;
    bool running;
    int sockfd, n;
    struct sockaddr_in serv_addr;
    GameInfo game;
    pthread_t * threads;
    pthread_mutex_t mutex;
} ClientGameInfo;

void SetupTerminal(struct termios *original);
void ResetTerminal(struct termios *original);
int NewGame(ClientGameInfo* info, int port);
int JoinGame(ClientGameInfo* info, int port, const char* ip);
void Run(ClientGameInfo* info);
void* DrawToClient(void* args);
void* ManageInputs(void* args);
