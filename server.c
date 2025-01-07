#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "comunication.h"

typedef struct ServerInfo{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
}ServerInfo;

typedef struct ServerPlayer{
    pthread_mutex_t * game_lock;
    pthread_cond_t * waitCondition;
    int * removedIndex;
    int player_id, index;
    int client_sock;
    GameInfo * game;    
    bool * waiting;
    bool * activePlayers;
    bool ended;
    bool paused;
} ServerPlayer;

typedef struct ServerSendingThread{
    ServerPlayer * players;
    int numOfPlayers;
}ServerSendingThread;

void* readPlayerDirection(void* args)
{
    ServerPlayer* serverPLayer = (ServerPlayer*) args;

    char buffer[256];
    int n;
	char lastCh = 'd';
	char ch;
	int running = 1;    
    printf("Player %d connected waiting 3s\n", serverPLayer->player_id);   

    pthread_mutex_lock(serverPLayer->game_lock);
    serverPLayer->index = AddPlayer(serverPLayer->game);
    int curNum = serverPLayer->game->numOfCurPLayers;
    if (serverPLayer->index == -1)
    {   
        printf("error adding player");
        return NULL;
    }
    *serverPLayer->waiting = true;
    pthread_mutex_unlock(serverPLayer->game_lock);

    sleep(3);

    pthread_mutex_lock(serverPLayer->game_lock);
    *serverPLayer->waiting = false;
    pthread_cond_broadcast(serverPLayer->waitCondition);
    pthread_mutex_unlock(serverPLayer->game_lock);

    while (running == 1) {    

        pthread_mutex_lock(serverPLayer->game_lock);
        while (*serverPLayer->waiting) {
            pthread_cond_wait(serverPLayer->waitCondition, serverPLayer->game_lock);
        }
        pthread_mutex_unlock(serverPLayer->game_lock);

        bzero(buffer, 256);
        n = read(serverPLayer->client_sock, buffer, 255); 
        if (n < 0) {
            perror("Error reading from socket");
            break;
        }

		ch = buffer[0];  
        pthread_mutex_lock(serverPLayer->game_lock);          
        if(ch != lastCh)
        {
            if (ch == '\n' && serverPLayer->game->players[serverPLayer->index].player.dead) {
            ResetPlayerInGame(serverPLayer->game, serverPLayer->index);
            } else if (ch == 'q' && serverPLayer->game->players[serverPLayer->index].player.dead) {
                printf("Q\n");
                running = 0;
                *serverPLayer->removedIndex = RemovePlayer(serverPLayer->game, &serverPLayer->game->players[serverPLayer->index]);
            } else if(ch == 'p') {
                serverPLayer->paused = !serverPLayer->paused;
                usleep(100000);
            }else if (ch == 'w' && !serverPLayer->paused) {
                TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, UP);
            } else if (ch == 's' && !serverPLayer->paused) {
                TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, DOWN);
            } else if (ch == 'a' && !serverPLayer->paused) {
                TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, LEFT);
            } else if (ch == 'd' && !serverPLayer->paused) {
                TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, RIGHT);
            }
        }      
        
        if(!serverPLayer->paused)
            MovePlayer(serverPLayer->game, &serverPLayer->game->players[serverPLayer->index]);            
	        
        if(serverPLayer->index > *serverPLayer->removedIndex && curNum > serverPLayer->game->numOfCurPLayers)
        {
            serverPLayer->index--;
            curNum = serverPLayer->game->numOfCurPLayers;
        }   
        //DrawGame(serverPLayer->game,-1);
        pthread_mutex_unlock(serverPLayer->game_lock);
        lastCh = ch;
    }

    close(serverPLayer->client_sock);    
    serverPLayer->activePlayers[serverPLayer->player_id] = false;
    printf("Player %d disconnected\n", serverPLayer->player_id);   
    pthread_exit(NULL);
}

void* sendGameData(void* args)
{
    ServerSendingThread* data = (ServerSendingThread*)args;
    bool running = true;

    while(running)
    {      
        time_t start = time(NULL);
        bool tmp = false;
        pthread_mutex_lock(data->players[0].game_lock);
        for (int i = 0; i < data->numOfPlayers; ++i)
        {
            if(!data->players[0].activePlayers[i])
                continue;
            if (!data->players[i].ended)
            {
                running = true;
                tmp = true;
                char* buff;
                size_t bufferSize = SerializeServerMessage(&buff, data->players[i].game, data->players[i].index);
                    
                send(data->players[i].client_sock, buff, bufferSize, 0);
                free(buff);
            }
            else if(!tmp)
            {
                running = false;
            }
        }
        pthread_mutex_unlock(data->players[0].game_lock);
        usleep(200000);//BEZ TOHO -> SEG FAULT U KLIENTA
        data->players[0].game->runningTime += (time(NULL) - start);
    }  
}

int init(ServerInfo* serverInfo,int serverSocket, int numOfPlayers) {
    int n;
    char buffer[256];

    bzero((char*)&serverInfo->serv_addr, sizeof(serverInfo->serv_addr));
    serverInfo->serv_addr.sin_family = AF_INET;
    serverInfo->serv_addr.sin_addr.s_addr = INADDR_ANY;
    int newSocket = serverSocket;
    serverInfo->serv_addr.sin_port = htons(newSocket);

    serverInfo->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverInfo->sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    while (bind(serverInfo->sockfd, (struct sockaddr*)&serverInfo->serv_addr, sizeof(serverInfo->serv_addr)) < 0) {
        printf("Error binding socket address\n");
        newSocket++;
        printf("New socket [%d]\n", newSocket);
        serverInfo->serv_addr.sin_port = htons(newSocket);
    }

    listen(serverInfo->sockfd, numOfPlayers);  // Čakaj na pripojenie až `numOfPlayers` hráčov
    serverInfo->cli_len = sizeof(serverInfo->cli_addr);
    return 0;
}

//port, numOfPLayers(1-n), width(5-n), height(5-n), timeEnd()
int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    if (argc < 5) {
        fprintf(stderr,"usage %s port, numOfPLayers, width, height, timeEnd(optional)\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int removedIndex = 141006540;
    pthread_mutex_t game_lock;
    pthread_cond_t waitCond;
    GameInfo game;
    int numOfPlayers = atoi(argv[2]);
    if (numOfPlayers < 1)
    {
        printf("%d must be atleast 1", numOfPlayers);
        return EXIT_FAILURE;
    }
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    if (width < 5 || height < 5)
    {
        printf("%d and %d  both must be atleast 5", width, height);
        return EXIT_FAILURE;
    }
    int timeEnd = 0;
    if (argc == 6)
    {
        timeEnd = atoi(argv[5]);
        timeEnd = timeEnd < 1 ? 0 : timeEnd;
    }

    CreateGame(&game, numOfPlayers, width, height, timeEnd);

    pthread_mutex_init(&game_lock,NULL);
    pthread_cond_init(&waitCond,NULL);

    ServerInfo serverInfo;
    init(&serverInfo, atoi(argv[1]), numOfPlayers);//init socket

    bool activePlayers[numOfPlayers];
    pthread_t threads[numOfPlayers + 1]; //players + sending;
    int currentPlayer = 0;
    ServerPlayer players[numOfPlayers];
    ServerSendingThread data;
    data.players = players;
    data.numOfPlayers = numOfPlayers;
    bool waiting = true;
    for(int i = 0; i < numOfPlayers; ++i)
    {
        players[i].game = &game;  
        players[i].removedIndex = &removedIndex;  
        players[i].ended = false;
        players[i].paused = false;
        players[i].activePlayers = activePlayers;
        players[i].game_lock = &game_lock;
        players[i].waitCondition = &waitCond;
        
        players[i].waiting = &waiting;

        activePlayers[i] = false;
    }

    if (pthread_create(&threads[numOfPlayers], NULL, &sendGameData, &data) != 0) {
        perror("Error creating thread");
        return EXIT_FAILURE;
    }

    while (1) {
        int newsockfd = accept(serverInfo.sockfd, (struct sockaddr *)&serverInfo.cli_addr, &serverInfo.cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accepting");
            continue;
        }

        currentPlayer = -1;
        for (int i = 0; i < numOfPlayers; i++) {
            if (!activePlayers[i]) {
                currentPlayer = i;
                break;
            }
        }

        if (currentPlayer == -1) {
            printf("Server full. Connection rejected.\n");
            close(newsockfd);
            continue;
        }

        players[currentPlayer].player_id = currentPlayer;
        players[currentPlayer].client_sock = newsockfd;

        activePlayers[currentPlayer] = true;

        if (pthread_create(&threads[currentPlayer], NULL, &readPlayerDirection, &players[currentPlayer]) != 0) {
            perror("Error creating thread");
            activePlayers[currentPlayer] = false;
            close(newsockfd);
            continue;
        }
    }

    for (int i = 0; i < numOfPlayers; i++) {
        pthread_join(threads[i], NULL);
    }

    close(serverInfo.sockfd);
    RemoveGame(&game);
    pthread_mutex_destroy(&game_lock);  
    pthread_cond_destroy(&waitCond);
    return 0;
}
