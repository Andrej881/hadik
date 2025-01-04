#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <termios.h>    // pre termios
#include <time.h>       // pre srand()
#include <fcntl.h>

#define MAX_BUF 256

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

void draw_game(int x1, int y1, int x2, int y2) {
    system("clear");
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == y1 && j == x1) {
                printf("O"); // Hadík
            } 
            else if(i == y2 && j == x2) {
                printf("#");    
            }
            else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void error(const char *msg) {
    perror(msg);
    exit(1);
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
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Prevod hostiteľa na IP adresu
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        error("ERROR invalid host");
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    printf("Connected to server\n");
	
	int x1,y1;
    int x2,y2;
    int id;
	char lastCh = 'd'; 
    int running = 1;
    while (running == 1) {
		draw_game(x1, y1, x2, y2);         
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
   
        usleep(200000);   
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
        }

        bzero(buffer, MAX_BUF);
        n = read(sockfd, buffer, MAX_BUF - 1);
        if (n < 0) {
            error("ERROR reading from socket");
        }
		sscanf(buffer, "%d %d %d %d %d",&id, &x1, &y1, &x2, &y2);
        printf("%d %d %d %d %d\n",id, x1, y1, x2, y2);
    }

    close(sockfd);
    reset_terminal(&original_settings);
    return 0;
}
