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

#include "game.h"

typedef struct ServerInfo{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
}ServerInfo;

typedef struct ServerPlayer{
    int player_id;
    int client_sock;
    GameInfo * game;
} ServerPlayer;

int removedIndex = 141006540;
sem_t game_lock;  // Semafor pre synchronizáciu

void* play_game(void* arg) {
    ServerPlayer* serverPLayer = (ServerPlayer*)arg;  // Získanie informácií o hráčovi
    char buffer[256];
    int n;
	char lastCh = 'd';
	char ch;
	int running = 1;
    printf("Player %d connected\n", serverPLayer->player_id);
    sem_wait(&game_lock);
    int index = AddPlayer(serverPLayer->game);
    int curNum = serverPLayer->game->numOfCurPLayers;
    if (index == -1)
    {   
        printf("error adding player");
        return NULL;
    }
    sem_post(&game_lock);

    while (running == 1) {
        // Čítanie pohybu od hráča
        bzero(buffer, 256);
        n = read(serverPLayer->client_sock, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            break;
        }

        // Uzamknutie semaforu pre bezpečný prístup k zdieľaným dátam
        sem_wait(&game_lock);
        printf("player %d index %d\n",serverPLayer->player_id, index);
		ch = buffer[0];
		if (ch == 'w' || ch == 's' || ch == 'a' || ch == 'd' || ch == 'q')
			lastCh = ch;
        // Spracovanie pohybu hráča
        if (lastCh == 'w') {
            TryChangeDir(&serverPLayer->game->players[index].player, UP);
        } else if (lastCh == 's') {
            TryChangeDir(&serverPLayer->game->players[index].player, DOWN);
        } else if (lastCh == 'a') {
            TryChangeDir(&serverPLayer->game->players[index].player, LEFT);
        } else if (lastCh == 'd') {
            TryChangeDir(&serverPLayer->game->players[index].player, RIGHT);
        }else if (lastCh == 'q') {
            running = 0;
            removedIndex = RemovePlayer(serverPLayer->game, &serverPLayer->game->players[index]);
        }
		
        MovePlayer(serverPLayer->game, &serverPLayer->game->players[index]);    

		printf("%c\t%d %d\n",buffer[0], serverPLayer->game->players[index].player.head.x, serverPLayer->game->players[index].player.head.y);
		fflush(NULL);

        // Synchronizácia stavu hry - poslanie nových pozícií všetkým hráčom
		char status[100]; 
		bzero(status,100);         
        if(index > removedIndex && curNum > serverPLayer->game->numOfCurPLayers)
        {
            index--;
            curNum = serverPLayer->game->numOfCurPLayers;
        }
		for (int i = 0; i < serverPLayer->game->numOfCurPLayers; i++) {
			char buffer[50];
			bzero(buffer, 50);
			snprintf(buffer, sizeof(buffer), "%d %d ", serverPLayer->game->players[i].player.head.x, serverPLayer->game->players[i].player.head.y);
			strncat(status, buffer, sizeof(status) - strlen(status) - 1); 
		}
        printf("writing to Player %d\n", serverPLayer->player_id);
		write(serverPLayer->client_sock, status, strlen(status));
        
        sem_post(&game_lock);
    }
    close(serverPLayer->client_sock);
    printf("Player %d disconnected\n", serverPLayer->player_id);      
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
        return 1;
    }

    GameInfo game;
    int numOfPlayers = atoi(argv[2]);
    if (numOfPlayers < 1)
    {
        printf("%d must be atleast 1", numOfPlayers);
        return 1;
    }
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    if (width < 5 || height < 5)
    {
        printf("%d and %d  both must be atleast 5", width, height);
        return 2;
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

    pthread_t threads[numOfPlayers];  // Vlákna pre hráčov
    int current_player = 0;
    ServerPlayer players[numOfPlayers];
    while (1) {
        // Akceptovanie pripojení klientov
        serverInfo.newsockfd = accept(serverInfo.sockfd, (struct sockaddr*)&serverInfo.cli_addr, &serverInfo.cli_len);
        if (serverInfo.newsockfd < 0) {
            perror("ERROR on accept");
            return 2;
        }

        // Priradenie hráča a jeho socketu
        players[current_player].player_id = current_player;
        players[current_player].client_sock = serverInfo.newsockfd;
        players[current_player].game = &game;

        // Vytvorenie vlákna pre každého pripojeného hráča
        if (pthread_create(&threads[current_player], NULL, play_game, &players[current_player]) != 0) {
            perror("Error creating thread");
            return 3;
        }

        current_player++;  // Prejdeme na ďalšieho hráča
        if (current_player >= numOfPlayers) {
            break;  // Ak je dostatok hráčov, ukončíme akceptovanie ďalších hráčov
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
