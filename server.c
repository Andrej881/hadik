#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "comunication.h"

typedef struct ServerInfo{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
}ServerInfo;

typedef struct ServerPlayer{
    sem_t * game_lock;
    int * removedIndex;
    int player_id, index;
    int client_sock;
    GameInfo * game;    
    bool * activePlayers;
    bool ended;
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
    printf("Player %d connected\n", serverPLayer->player_id);

    sem_wait(serverPLayer->game_lock);
    serverPLayer->index = AddPlayer(serverPLayer->game);
    int curNum = serverPLayer->game->numOfCurPLayers;
    if (serverPLayer->index == -1)
    {   
        printf("error adding player");
        return NULL;
    }
    sem_post(serverPLayer->game_lock);

    while (running == 1) {    
        bzero(buffer, 256);
        n = read(serverPLayer->client_sock, buffer, 255);      
        if (n < 0) {
            perror("Error reading from socket");
            break;
        }

        sem_wait(serverPLayer->game_lock);
		ch = buffer[0];
		if (ch == 'w' || ch == 's' || ch == 'a' || ch == 'd' || ch == 'q')
			lastCh = ch;
        if (lastCh == 'w') {
            TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, UP);
        } else if (lastCh == 's') {
            TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, DOWN);
        } else if (lastCh == 'a') {
            TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, LEFT);
        } else if (lastCh == 'd') {
            TryChangeDir(&serverPLayer->game->players[serverPLayer->index].player, RIGHT);
        }else if (lastCh == 'q') {
            running = 0;
            *serverPLayer->removedIndex = RemovePlayer(serverPLayer->game, &serverPLayer->game->players[serverPLayer->index]);
        }
        MovePlayer(serverPLayer->game, &serverPLayer->game->players[serverPLayer->index]);            
	        
        if(serverPLayer->index > *serverPLayer->removedIndex && curNum > serverPLayer->game->numOfCurPLayers)
        {
            serverPLayer->index--;
            curNum = serverPLayer->game->numOfCurPLayers;
        }   
        //DrawGame(serverPLayer->game);
        sem_post(serverPLayer->game_lock);
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
        bool tmp = false;
        sem_wait(data->players[0].game_lock);
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
        sem_post(data->players[0].game_lock);
        usleep(200000);//BEZ TOHO -> SEG FAULT U KLIENTA
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
    sem_t game_lock;
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
    int time = 0;
    if (argc == 6)
    {
        time = atoi(argv[5]);
        time = time < 1 ? 0 : time;
    }

    CreateGame(&game, numOfPlayers, width, height, time);

    sem_init(&game_lock, 0, 1);

    ServerInfo serverInfo;
    init(&serverInfo, atoi(argv[1]), numOfPlayers);//init socket

    bool activePlayers[numOfPlayers];
    pthread_t threads[numOfPlayers + 1];  // Vlákna pre hráčov
    int currentPlayer = 0;
    ServerPlayer players[numOfPlayers];
    ServerSendingThread data;
    data.players = players;
    data.numOfPlayers = numOfPlayers;

    for(int i = 0; i < numOfPlayers; ++i)
    {
        players[i].game = &game;  
        players[i].removedIndex = &removedIndex;  
        players[i].ended = false;
        players[i].activePlayers = activePlayers;
        players[i].game_lock = &game_lock;

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

        // Vytvorenie vlákna pre každého pripojeného hráča
        if (pthread_create(&threads[currentPlayer], NULL, &readPlayerDirection, &players[currentPlayer]) != 0) {
            perror("Error creating thread");
            activePlayers[currentPlayer] = false;
            close(newsockfd);
            continue;
        }
    }

    // Čakanie na dokončenie všetkých vlákien
    for (int i = 0; i < numOfPlayers; i++) {
        pthread_join(threads[i], NULL);
    }

    close(serverInfo.sockfd);
    RemoveGame(&game);
    sem_destroy(&game_lock);  // Zničenie semaforu po ukončení
    return 0;
}
