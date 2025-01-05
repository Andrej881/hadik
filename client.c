#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <termios.h>    // pre termios
#include <time.h>       // pre srand()
#include <fcntl.h>

#include "comunication.h"

#define MAX_BUF 10000

#define WIDTH 30
#define HEIGHT 15

void setup_terminal(struct termios *original) {
    struct termios new_settings;
    tcgetattr(0, original);              // Uloženie pôvodných nastavení
    new_settings = *original;
    new_settings.c_lflag &= ~(ICANON | ECHO); // Vypnutie kanonického režimu a echo
    tcsetattr(0, TCSANOW, &new_settings);    // Aplikácia zmien

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);// aby sa necakalo na input
}

void reset_terminal(struct termios *original) {
    tcsetattr(0, TCSANOW, original); // Obnovenie pôvodných nastavení
}

int main(int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUF];

	struct termios original_settings;
    setup_terminal(&original_settings); // Nastavenie terminálu
    srand(time(NULL)); // Inicializácia random generátora

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return 2;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Prevod hostiteľa na IP adresu
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("ERROR invalid host");
        return 3;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        return 4;
    }

    printf("Connected to server\n");
	
	int x1,y1;
    int x2,y2;
    int id;
	char lastCh = 'd'; 
    int running = 1;

    GameInfo game;
    CreateGame(&game, 2, 30, 15, 0);   

    while (running == 1) {      
		bzero(buffer, MAX_BUF);    
        char ch;
        if(read(STDIN_FILENO, &ch, 1) == 1)
		{
            lastCh = ch;
		}
        
        buffer[0] = lastCh;

        if (buffer[0] == 'q') {
            running = 0;  // Ukončenie programu
        }  
          
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket");
            return 5;
        }

        bzero(buffer, MAX_BUF);
        size_t bufferSize;
        // Receive the serialized data
        ssize_t test = recv(sockfd, buffer, MAX_BUF, 0);
        if (test <= 0) {
            printf("Failed to receive serialized data %ld", test);            
            exit(EXIT_FAILURE);
        }
        DeserializeServerMessage(buffer, &game);
        //PrintGameContent(&game);
        DrawGame(&game);
        //printf("head [%d, %d]\n", game.players[0].player.head.x, game.players[0].player.head.y);
        usleep(200000); 
    }

    close(sockfd);
    reset_terminal(&original_settings);
    return 0;
}
