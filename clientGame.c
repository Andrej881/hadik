#include "clientGame.h"

void SetupTerminal(struct termios *original) {
    struct termios new_settings;
    tcgetattr(0, original);             
    new_settings = *original;
    new_settings.c_lflag &= ~(ICANON | ECHO); 
    new_settings.c_cc[VMIN] = 1;             
    new_settings.c_cc[VTIME] = 0;  
    tcsetattr(0, TCSANOW, &new_settings);    
}

void ResetTerminal(struct termios *original) {
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

int NewGame(ClientGameInfo* info, int port)
{    
    int num;
    while (1) {
        printf("Write Num of players: \n");
        if (scanf("%d", &num) == 1) {
            if (num > 0) {
                break; 
            } else {
                printf("Must be greater than 0!\n");
            }
        } else {
            printf("Invalid input! Please enter a number.\n");
                
            while (getchar() != '\n');
        }
    }    

    int width;
    while (1) {
        printf("Write Width: \n");
        if (scanf("%d", &width) == 1) {
            if (width > 0) {
                break; 
            } else {
                printf("Must be greater than 0!\n");
            }
        } else {
            printf("Invalid input! Please enter a number.\n");
                
            while (getchar() != '\n');
        }
    }  

    int height;
    while (1) {
        printf("Write Height: \n");
        if (scanf("%d", &height) == 1) {
            if (height > 0) {
                break; 
            } else {
                printf("Must be greater than 0!\n");
            }
        } else {
            printf("Invalid input! Please enter a number.\n");
                
            while (getchar() != '\n');
        }
    }   

    int gameTime;
    while (1) {
        printf("Write Time in which game should end(0 means it ends only when there are no players): \n");
        if (scanf("%d", &gameTime) == 1) {
            if (gameTime >= 0) {
                break; 
            } else {
                printf("Must be greater or equal to 0!\n");
            }
        } else {
            printf("Invalid input! Please enter a number.\n");
                
            while (getchar() != '\n');
        }
    }   

    char gameWalls[2];
    bzero(&gameWalls,2);
    printf("Write 1 if you want map with walls: \n");
    scanf("%s", &gameWalls);       
    
    char numStr[12], widthStr[12], heightStr[12], portStr[12], gameTimeStr[12];
    snprintf(numStr, sizeof(numStr), "%d", num);
    snprintf(widthStr, sizeof(widthStr), "%d", width);
    snprintf(heightStr, sizeof(heightStr), "%d", height);
    snprintf(portStr, sizeof(portStr), "%d", port);
    snprintf(gameTimeStr, sizeof(gameTimeStr), "%d", gameTime);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // Server
        printf("Spúšťam server v detskom procese...\n");

        
        char *server_path = "./server";
        char *args[] = {server_path, portStr, numStr, widthStr, heightStr, gameTimeStr, gameWalls, NULL};

        if (execvp(server_path, args) < 0) {
            perror("execvp failed");
            return -2;
        }
    } else {
        // Klient
        bool contain = gameWalls[0] == '1';
        CreateGame(&info->game, num, width, height, gameTime, contain);
        usleep(2000000);
        return JoinGame(info, port, "127.0.0.1");
    }

    return 0;
}

int JoinGame(ClientGameInfo* info, int port, const char* ip)
{    
    CreateGame(&info->game, 5, 30, 15, 0, false);

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

void* DrawToClient(void* args)
{   
    ClientGameInfo* info = (ClientGameInfo*) args;
    char buffer[MAX_BUF];    
    int running = true;
    while(running)
    {  
        usleep(166670);
        bzero(buffer, MAX_BUF);
        size_t bufferSize;
        // Receive the serialized data
        ssize_t test = -1;
        pthread_mutex_lock(&info->mutex);
        if(!info->running)
        {            
            running = info->running; 
            pthread_mutex_unlock(&info->mutex);
            continue;
        }
        test = recv(info->sockfd, buffer, MAX_BUF, 0);
        pthread_mutex_unlock(&info->mutex);
        if (test <= 0) {
            printf("Failed to receive serialized data %ld", test);            
            return NULL;
        }
        int playerIndex;
        DeserializeServerMessage(buffer, &info->game, &playerIndex);
        //PrintGameContent(&info->game);
        info->game.players[playerIndex].index = playerIndex;
        if (playerIndex == 0)
        {
            for (int i = 1; i < info->game.numOfCurPLayers; ++i)
            {
                info->game.players[i].index = -1;                
            }
        }
        DrawGame(&info->game, playerIndex);

        pthread_mutex_lock(&info->mutex);
        if (info->dead != info->game.players[playerIndex].player.dead)
            info->dead = !info->dead;
        running = info->running; 
        pthread_mutex_unlock(&info->mutex);
    }
    
}

void* ManageInputs(void* args)
{
    ClientGameInfo* info = (ClientGameInfo*) args;
    char buffer[MAX_BUF];
	char lastCh = 'd'; 
    int n;
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);//aby sa v hre necakalo na input
    while (info->running) {  
        usleep(200000);  
        char ch;
        if(read(STDIN_FILENO, &ch, 1) == 1)
		{
            lastCh = ch;
		}        
		bzero(buffer, MAX_BUF);  
        buffer[0] = lastCh;
        if (buffer[0] == 'q') {
            pthread_mutex_lock(&info->mutex);
            if (info->dead)
                info->running = false;  // Ukončenie programu            
            pthread_mutex_unlock(&info->mutex);
        } 
        n = write(info->sockfd, buffer, strlen(buffer));
        if (n < 0) {            
            perror("ERROR writing to socket");
            return NULL;
        }
    }
}

void Run(ClientGameInfo* info)
{
    printf("Running\n");
    info->running = true;
    info->dead = false;

    pthread_mutex_init(&info->mutex, NULL);
    
    SetupTerminal(&info->original);

    info->threads = malloc(2 * sizeof(pthread_t));

    pthread_create(&info->threads[0], NULL, &ManageInputs, info);
    usleep(200000);
    pthread_create(&info->threads[1], NULL, &DrawToClient, info);
    
    for(int i = 0; i < 2; ++i)
    {
        pthread_join(info->threads[i], NULL);
    }

    ResetTerminal(&info->original);
    free(info->threads);
    info->threads = NULL;
    pthread_mutex_destroy(&info->mutex);
}