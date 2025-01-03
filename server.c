#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_PLAYERS 2  // Maximálny počet hráčov (pre tento príklad 2)
#define WIDTH 30
#define HEIGHT 15

int sockfd, newsockfd;
struct sockaddr_in serv_addr, cli_addr;
socklen_t cli_len;

typedef struct {
    int x, y;
    char direction; // Pohyb hráča
    int player_id;
    int client_sock;
} Player;

Player players[MAX_PLAYERS];  // Uchovávanie hráčov
sem_t game_lock;  // Semafor pre synchronizáciu

void init_game() {
    // Inicializácia hráčov na rôzne pozície
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].x = i * 10;  // Začiatok hráča 0 na (0, 0), hráč 1 na (10, 0)
        players[i].y = 0;
        players[i].direction = ' ';
        players[i].player_id = i;
    }
    
    // Inicializácia semaforu
    sem_init(&game_lock, 0, 1);  // Binárny semafor (mutex)
}

void* play_game(void* arg) {
    Player* player = (Player*)arg;  // Získanie informácií o hráčovi
    char buffer[256];
    int n;
	char lastCh = 'd';
	char ch;
	int running = 1;
    printf("Player %d connected\n", player->player_id);

    while (running == 1) {
        // Čítanie pohybu od hráča
        bzero(buffer, 256);
        n = read(player->client_sock, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            break;
        }

        // Uzamknutie semaforu pre bezpečný prístup k zdieľaným dátam
        sem_wait(&game_lock);
		ch = buffer[0];
		if (ch == 'w' || ch == 's' || ch == 'a' || ch == 'd' || ch == 'q')
			lastCh = ch;
        // Spracovanie pohybu hráča
        if (lastCh == 'w') {
            player->y--;
        } else if (lastCh == 's') {
            player->y++;
        } else if (lastCh == 'a') {
            player->x--;
        } else if (lastCh == 'd') {
            player->x++;
        }else if (lastCh == 'q') {
            running = 0;
        }
		
		player->x = player->x < 0 ? 0 : player->x;
		player->y = player->y < 0 ? 0 : player->y;
		player->x = player->x > WIDTH ? WIDTH-1 : player->x;
		player->y = player->y > HEIGHT ? HEIGHT-1 : player->y;

		printf("%c\t%d %d\n",buffer[0],player->x,player->y);
		fflush(NULL);

        // Synchronizácia stavu hry - poslanie nových pozícií všetkým hráčom
		char status[100]; // Increase size to accommodate all players
		bzero(status,100);  // Initialize as an empty string

		for (int i = 0; i < MAX_PLAYERS; i++) {
			char buffer[50];
			bzero(buffer, 50);
			snprintf(buffer, sizeof(buffer), "%d %d ", players[i].x, players[i].y);
			strncat(status, buffer, sizeof(status) - strlen(status) - 1); // Safely append to status
		}	
		write(player->client_sock, status, strlen(status));
        // Uvoľnenie semaforu po aktualizácii stavu
        sem_post(&game_lock);
    }
    close(player->client_sock);
    printf("Player %d disconnected\n", player->player_id);
    pthread_exit(NULL);  // Ukončenie vlákna
}

int init(int serverSocket, int numOfPlayers) {
    int n;
    char buffer[256];

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    int newSocket = serverSocket;
    serv_addr.sin_port = htons(newSocket);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    while (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error binding socket address\n");
        newSocket++;
        printf("New socket [%d]\n", newSocket);
        serv_addr.sin_port = htons(newSocket);
    }

    listen(sockfd, numOfPlayers);  // Čakaj na pripojenie až `numOfPlayers` hráčov
    cli_len = sizeof(cli_addr);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr,"usage %s port numOfPlayers\n", argv[0]);
        return 1;
    }

    // Inicializácia servera a semaforu pre synchronizáciu
    init_game();
    init(atoi(argv[1]), atoi(argv[2]));

    pthread_t threads[MAX_PLAYERS];  // Vlákna pre hráčov
    int current_player = 0;

    while (1) {
        // Akceptovanie pripojení klientov
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            return 2;
        }

        // Priradenie hráča a jeho socketu
        players[current_player].client_sock = newsockfd;

        // Vytvorenie vlákna pre každého pripojeného hráča
        if (pthread_create(&threads[current_player], NULL, play_game, &players[current_player]) != 0) {
            perror("Error creating thread");
            return 3;
        }

        current_player++;  // Prejdeme na ďalšieho hráča
        if (current_player >= MAX_PLAYERS) {
            break;  // Ak je dostatok hráčov, ukončíme akceptovanie ďalších hráčov
        }
    }

    // Čakanie na dokončenie všetkých vlákien
    for (int i = 0; i < MAX_PLAYERS; i++) {
        pthread_join(threads[i], NULL);
    }
    close(sockfd);
    sem_destroy(&game_lock);  // Zničenie semaforu po ukončení
    return 0;
}
