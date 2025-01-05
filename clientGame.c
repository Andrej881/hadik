#include "clientGame.h"

void NewGame(ClientGameInfo* info){}

int JoinGame(ClientGameInfo* info, int port, const char* ip)
{
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);//aby sa v hre necakalo na input

    info->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (info->sockfd < 0) {
        printf("ERROR opening socket");
        return -1;
    }

    bzero((char *) &info->serv_addr, sizeof(info->serv_addr));
    info->serv_addr.sin_family = AF_INET;
    info->serv_addr.sin_port = htons(port);

    // Prevod hostiteľa na IP adresu
    if (inet_pton(AF_INET, ip, &info->serv_addr.sin_addr) <= 0) {
        printf("ERROR invalid host");
        return -2;
    }

    if (connect(info->sockfd, (struct sockaddr *)&info->serv_addr, sizeof(info->serv_addr)) < 0) {
        printf("ERROR connecting");
        return -3;
    }

    printf("Connected to server\n");
    return 0;
}

void Run(ClientGameInfo* info)
{
    char buffer[MAX_BUF];

	char lastCh = 'd'; 
    int running = 1;

    CreateGame(&info->game, 2, 30, 15, 0);   

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
          
        info->n = write(info->sockfd, buffer, strlen(buffer));
        if (info->n < 0) {            
            //reset_terminal(&info->original_settings);
            perror("ERROR writing to socket");
            return;
        }

        bzero(buffer, MAX_BUF);
        size_t bufferSize;
        // Receive the serialized data
        ssize_t test = recv(info->sockfd, buffer, MAX_BUF, 0);
        if (test <= 0) {
            //reset_terminal(&info->original_settings);
            printf("Failed to receive serialized data %ld", test);            
            return;
        }
        DeserializeServerMessage(buffer, &info->game);
        //PrintGameContent(&info->game);
        DrawGame(&info->game);
        //printf("head [%d, %d]\n", info->game.players[0].player.head.x, info->game.players[0].player.head.y);
        //usleep(200000); 
    }

}