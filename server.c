#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "comunication.h"

typedef struct Message
{
    Direction dir[2];
    int num;
} Message;

typedef struct ServerInfo{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
}ServerInfo;

typedef struct ServerPlayer{
    pthread_mutex_t * gameMutex;
    pthread_cond_t * waitCondition;
    int * removedIndex;
    int player_id, index;
    int client_sock;
    GameInfo * game;    
    bool * waiting;
    bool * activePlayers;
    bool ended;
    bool paused;
    bool * end;
    Direction direction;
} ServerPlayer;

typedef struct ServerSendingThread{
    ServerPlayer * players;
    int numOfPlayers;
}ServerSendingThread;

void* readPlayerDirection(void* args)
{
    ServerPlayer* serverPLayer = (ServerPlayer*) args;

    char buffer[10000];
    bzero(buffer, 10000);
    int n;
	char lastCh = 'd';
	char ch;
	int running = 1;    
    printf("Player %d connected waiting 3s\n", serverPLayer->player_id);   

    pthread_mutex_lock(serverPLayer->gameMutex);

    SerializeInitMessage(buffer, serverPLayer->game);
    send(serverPLayer->client_sock, buffer, 1000, 0);
    
    serverPLayer->index = AddPlayer(serverPLayer->game);
    serverPLayer->direction = serverPLayer->game->players[serverPLayer->index].player.curDir;
    serverPLayer->paused = false;
    int curNum = serverPLayer->game->numOfCurPLayers;
    if (serverPLayer->index == -1)
    {   
        printf("error adding player");
        return NULL;
    }
    *serverPLayer->waiting = true;
    pthread_mutex_unlock(serverPLayer->gameMutex);

    sleep(3);

    pthread_mutex_lock(serverPLayer->gameMutex);
    *serverPLayer->waiting = false;
    pthread_cond_broadcast(serverPLayer->waitCondition);
    pthread_mutex_unlock(serverPLayer->gameMutex);

    while (running == 1) {    

        pthread_mutex_lock(serverPLayer->gameMutex);
        if(*serverPLayer->end)
        {        
            pthread_mutex_unlock(serverPLayer->gameMutex);
            break;
        }
        while (*serverPLayer->waiting) {
            pthread_cond_wait(serverPLayer->waitCondition, serverPLayer->gameMutex);
        }
        pthread_mutex_unlock(serverPLayer->gameMutex);

        bzero(buffer, 10000);
        n = read(serverPLayer->client_sock, buffer, 10000); 
        if (n < 0) {
            perror("Error reading from socket");
            break;
        }

		ch = buffer[0];  
        pthread_mutex_lock(serverPLayer->gameMutex);          
        if(ch != lastCh)
        {
            if (ch == '\n' && serverPLayer->game->players[serverPLayer->index].player.dead) {
                ResetPlayerInGame(serverPLayer->game, serverPLayer->index);
                lastCh = ' ';  
            } else if (ch == 'q' && serverPLayer->game->players[serverPLayer->index].player.dead) {
                printf("Q\n");
                running = 0;
                *serverPLayer->removedIndex = RemovePlayer(serverPLayer->game, &serverPLayer->game->players[serverPLayer->index]);
            } else if(ch == 'p') {
                serverPLayer->paused = !serverPLayer->paused;
                if(serverPLayer->paused == false)
                {
                    *serverPLayer->waiting = true;
                    sleep(3);
                    *serverPLayer->waiting = false;
                }
            }else if (ch == 'w' && !serverPLayer->paused) {
                serverPLayer->direction = UP;
            } else if (ch == 's' && !serverPLayer->paused) {
                serverPLayer->direction = DOWN;
            } else if (ch == 'a' && !serverPLayer->paused) {
                serverPLayer->direction = LEFT;
            } else if (ch == 'd' && !serverPLayer->paused) {
                serverPLayer->direction = RIGHT;
            }
        }                    
	        
        if(serverPLayer->index > *serverPLayer->removedIndex && curNum > serverPLayer->game->numOfCurPLayers)
        {
            serverPLayer->index--;
            curNum = serverPLayer->game->numOfCurPLayers;
        }   
        //DrawGame(serverPLayer->game,-1);
        pthread_mutex_unlock(serverPLayer->gameMutex);
        lastCh = ch;     
    }

    pthread_mutex_lock(serverPLayer->gameMutex);
    close(serverPLayer->client_sock);    
    serverPLayer->activePlayers[serverPLayer->player_id] = false;
    pthread_mutex_unlock(serverPLayer->gameMutex);
    printf("Player %d disconnected\n", serverPLayer->player_id);   
    pthread_exit(NULL);
}

void* sendGameData(void* args)
{
    sleep(1);
    ServerSendingThread* data = (ServerSendingThread*)args;
    bool running = true;
    time_t start, passed = 0;
    while(running)
    {      
        start = time(NULL);
        bool tmp = false;
        
        pthread_mutex_lock(data->players[0].gameMutex);  
        /*while (*data->players[0].waiting) {
            pthread_cond_wait(data->players[0].waitCondition, data->players[0].gameMutex);
        }*/
        for (int i = 0; i < data->numOfPlayers; ++i)
        {
            if (!data->players[i].paused && data->players[0].activePlayers[i] && *data->players[0].waiting == false)
            {                
                TryChangeDir(&data->players[i].game->players[data->players[i].index].player, data->players[i].direction);
                MovePlayer(data->players[i].game, &data->players[i].game->players[data->players[i].index]);
            }
        }              
        
        if(*data->players[0].end)
        {            
            pthread_mutex_unlock(data->players[0].gameMutex); 
            break;
        }           
            
        char* buff = NULL;
        size_t bufferSize;
        if(data->players[0].game->running == false)
        {
            for (int i = 0; i < data->numOfPlayers; ++i)
            {
                if(!data->players[0].activePlayers[i])
                    continue;            
                char buf[2 + sizeof(int)];
                bzero(buf, 2+sizeof(int));
                buf[0] = 'Q';
                memcpy(buf + 1, &data->players[i].index, sizeof(int));
                send(data->players[i].client_sock, buf, 2 + sizeof(int), 0);              
            }
            *data->players[0].end = true;
            pthread_mutex_unlock(data->players[0].gameMutex);
            break;
        }
        //DrawGame(data->players[i].game, -1);
        for (int i = 0; i < data->numOfPlayers; ++i)
        {                
            if(!data->players[0].activePlayers[i])
                continue;  
            else if(buff == NULL)               
                bufferSize = SerializeServerMessage(&buff, data->players[0].game);  
                                 
            if (!data->players[i].ended)
            {
                running = true;
                tmp = true;
                char* ptr = buff;
                ptr += bufferSize - sizeof(int);
                memcpy(ptr, &data->players[i].index, sizeof(int));
                //DrawGame(data->players[i].game, -1);
                send(data->players[i].client_sock, buff, bufferSize, 0);                
            }
            else if(!tmp)
            {
                running = false;
            }
        }   
        if(buff != NULL)
            free(buff);
        AddTime(data->players[0].game, passed);
        pthread_mutex_unlock(data->players[0].gameMutex);
        usleep(200000);//BEZ TOHO -> SEG FAULT U KLIENTA
        passed = time(NULL) - start;
    } 
}

int init(ServerInfo* serverInfo,int serverSocket, int numOfPlayers) {
    int n;
    char buffer[256];

    bzero((char*)&serverInfo->serv_addr, sizeof(serverInfo->serv_addr));
    serverInfo->serv_addr.sin_family = AF_INET;
    serverInfo->serv_addr.sin_addr.s_addr = INADDR_ANY;
    serverInfo->serv_addr.sin_port = htons(serverSocket);

    serverInfo->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverInfo->sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (bind(serverInfo->sockfd, (struct sockaddr*)&serverInfo->serv_addr, sizeof(serverInfo->serv_addr)) < 0) {
        printf("Error binding socket address\n");
        return 2;
    }

    listen(serverInfo->sockfd, numOfPlayers);  // Čakaj na pripojenie až `numOfPlayers` hráčov
    serverInfo->cli_len = sizeof(serverInfo->cli_addr);
    return 0;
}

//port, numOfPLayers(1-n), width(5-n), height(5-n), timeEnd()
int main(int argc, char *argv[]) {
    
    bool fileLoad = false;
    srand(time(NULL));
    if(argc == 3)
    {
        fileLoad = true;
    }
    else if (argc < 5) {
        fprintf(stderr,"usage %s port, filePath || numOfPLayers, width, height, timeEnd(optional), walls(optional-timeEnd mast be set to 0 because its before)\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int removedIndex = 141006540;
    pthread_mutex_t gameMutex;
    pthread_cond_t waitCond;
    GameInfo game;
    int numOfPlayers;
    if(!fileLoad)
    {
        numOfPlayers = atoi(argv[2]);
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
        if (argc >= 6)
        {
            timeEnd = atoi(argv[5]);
            timeEnd = timeEnd < 1 ? 0 : timeEnd;
        }
        int walls = 0;        
        walls = atoi(argv[6]);

        CreateGame(&game, numOfPlayers, width, height, timeEnd, walls);
    }
    else
    {
        int test = CreateGameFromFile(&game, argv[2]);
        
        printf("TEST [%d] %d\n", test, game.numOfPlayers);
        if(test != 0)
        {
            printf("Failed to create Game from file [%d]\n", test);
            return EXIT_FAILURE;
        }
        numOfPlayers = game.numOfPlayers;
    }
    printf("num of players: %d", numOfPlayers);

    pthread_mutex_init(&gameMutex,NULL);
    pthread_cond_init(&waitCond,NULL);

    ServerInfo serverInfo;
    if(init(&serverInfo, atoi(argv[1]), numOfPlayers) != 0)//init socket
    {
        printf("Error creating server on port %d", atoi(argv[1]));
        return(EXIT_FAILURE);
    }

    bool activePlayers[numOfPlayers];
    pthread_t threads[numOfPlayers + 1]; //players + sending;
    int currentPlayer = 0;
    ServerPlayer players[numOfPlayers];
    ServerSendingThread data;
    bool end = false;
    data.players = players;
    data.numOfPlayers = numOfPlayers;
    bool waiting = true;
    for(int i = 0; i < numOfPlayers; ++i)
    {
        players[i].game = &game;  
        players[i].removedIndex = &removedIndex;  
        players[i].ended = false;
        players[i].paused = true;
        players[i].activePlayers = activePlayers;
        players[i].gameMutex = &gameMutex;
        players[i].waitCondition = &waitCond;
        players[i].end = &end;

        players[i].waiting = &waiting;

        activePlayers[i] = false;
    }

    if (pthread_create(&threads[numOfPlayers], NULL, &sendGameData, &data) != 0) {
        perror("Error creating thread\n");
        return EXIT_FAILURE;
    }

    bool timerActive = false;
    bool hasPlayerJoined = false; // Flag to ensure the timer only starts after the first player joins
    time_t timerStart = 0;
    int maxActiveThreads = 0;
    while (1) {
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(serverInfo.sockfd, &readfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Wait for incoming connection or timeout
        int activity = select(serverInfo.sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("Error in select\n");
            break;
        }

        // Timer check: If active and no players joined in 10 seconds, shut down the server
        if (timerActive || end) {
            time_t currentTime = time(NULL);
            if (currentTime - timerStart >= 10) {
                pthread_mutex_lock(&gameMutex);
                end = true;
                pthread_mutex_unlock(&gameMutex);
                printf("No players connected for 10 seconds or Game ended. Server shutting down.\n");
                break;
            }
        }

        // Check for new connection
        if (FD_ISSET(serverInfo.sockfd, &readfds)) {
            int newsockfd = accept(serverInfo.sockfd, (struct sockaddr *)&serverInfo.cli_addr, &serverInfo.cli_len);
            if (newsockfd < 0) {
                perror("ERROR on accepting\n");
                continue;
            }

            // Find an available player slot
            currentPlayer = -1;
            for (int i = 0; i < numOfPlayers; i++) {
                if (!activePlayers[i]) {
                    currentPlayer = i;
                    break;
                }
            }

            if (currentPlayer == -1) {
                printf("Server full. Connection rejected.\n");
                char buf[2] = {'Q', '\0'};
                send(newsockfd, buf, 2, 0);
                close(newsockfd);
                continue;
            }

            maxActiveThreads = maxActiveThreads < currentPlayer ? currentPlayer : maxActiveThreads;
            players[currentPlayer].player_id = currentPlayer;
            players[currentPlayer].client_sock = newsockfd;

            activePlayers[currentPlayer] = true;
            hasPlayerJoined = true; // Mark that at least one player has joined

            // Reset timer if a new player connects
            if (timerActive) {
                timerActive = false;
                printf("New player connected. Timer reset.\n");
            }

            if (pthread_create(&threads[currentPlayer], NULL, &readPlayerDirection, &players[currentPlayer]) != 0) {
                perror("Error creating thread\n");
                close(newsockfd);
                continue;
            }
            pthread_detach(threads[currentPlayer]);
        }

        // Check if all players have disconnected        
        int activeCount = 0;
        for (int i = 0; i < numOfPlayers; i++) {
            
            if (activePlayers[i]) {
                activeCount++;
            }
        }

        if (hasPlayerJoined && activeCount == 0 && !timerActive) {
            // Start the timer only if at least one player has joined before
            timerStart = time(NULL);
            timerActive = true;
            printf("No active players. Timer started.\n");
        }
    }

    printf("[SendData] %ld\n", threads[numOfPlayers]);
    pthread_join(threads[numOfPlayers], NULL);

    close(serverInfo.sockfd);
    SaveGameSetUp(&game,"LastGame");
    RemoveGame(&game);
    pthread_mutex_destroy(&gameMutex);  
    pthread_cond_destroy(&waitCond);
    return 0;
}
