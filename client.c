#include "clientGame.h"

#define SCREEN_WIDTH 30
#define SCREEN_HEIGHT 11

void PrintBorder(int i, int position)
{
    if((i < 3 && position == 1) || (i > 3 && i < 7 && position == 2) || (i > 7 && position == 3))
    {
        printf("@");
    }
    else
    {
        printf("#");
    }
}

int GameMenu(struct termios * original)
{
    int position = 2;
    
    SetupTerminal(original); 
    while (1)
    {
        system("clear");
        for(int i = 0; i < SCREEN_HEIGHT; ++i)
        {
            for(int j = 0; j < SCREEN_WIDTH; ++j)
            {
                if(i == 0 || i == 2 || i == 4 || i == 6 || i == 8 || i == 10)
                {
                    PrintBorder(i, position);
                }
                else if(i != 3 && i != 7)
                {
                    if (j == 0 || j == SCREEN_WIDTH - 1)
                    {
                        PrintBorder(i, position);
                    }
                    else 
                    {
                        switch (i)
                        {
                        case 1:
                            if (j == 10)
                            {
                                printf("NEW GAME");
                                j += 7;
                            }  
                            else
                            {                                
                                printf(" ");
                            }
                            break;
                        case 5:
                            if (j == 10)
                            {
                                printf("JOIN GAME");
                                j += 8;
                            }      
                            else
                            {                                
                                printf(" ");
                            }                            
                            break;
                        case 9:
                            if (j == 10)
                            {
                                printf("EXIT");
                                j += 3;
                            }    
                            else
                            {                                
                                printf(" ");
                            }
                            break;
                        }
                    }
                }     
                else
                {
                    printf(" ");
                }

            }
            printf("\n");
        }
        char ch;
        bool reading = true;
        while(reading)
        {
            if(read(STDIN_FILENO, &ch, 1) == 1)
            {
                reading = false;
                switch(ch)
                {
                case 'w':
                    position = position-1 >= 1 ? position - 1 : position;
                    break;
                case 's':
                    position = position + 1 <= 3 ? position + 1 : position;
                    break;
                case '\n'://enter
                    ResetTerminal(original);
                    return position;
                    break;
                default:
                    reading = true;
                    break;                
                }
            } 
        }
        
    }   
    ResetTerminal(original); 
}

int main(int argc, char *argv[]) {
    ClientGameInfo info;    
    srand(time(NULL));   
    
    switch(GameMenu(&info.original))
    {        
    case 1:
    {
        printf("Write Game Port: \n");
        int port;
        while (1) {
            if (scanf("%d", &port) == 1) {
                if (port >= 1024 && port <= 49151) {
                    break; // Platné portové číslo, ukonči cyklus
                } else {
                    printf("Port number must be between 1024 amd 49151!\n");
                }
            } else {
                printf("Invalid input! Please enter a number.\n");
                
                while (getchar() != '\n');
            }
            printf("Write Game Port: \n");
        }
        int test = NewGame(&info, port);
        if(test < 0)
        {
            printf("Failed to create Game or Join the created game returned[%d]\n", test);
            return EXIT_FAILURE;
        };     
        Run(&info);
        break;
    }        
    case 2:
    {                
        int port;
        while (1) {
            printf("Write Game Port: \n");
            if (scanf("%d", &port) == 1) {
                if (port >= 1024 && port <= 49151) {
                    break; 
                } else {
                    printf("Port number must be between 1024 amd 49151!\n");
                }
            } else {
                printf("Invalid input! Please enter a number.\n");
                // Vyčistenie vstupného bufferu
                while (getchar() != '\n');
            }
        }
        
        char ip[INET_ADDRSTRLEN];

        while (1) {            
            printf("Write IP: \n");
            if (scanf("%s", ip) != 1) { // Read up to 15 characters (+1 for null terminator)
                printf("Invalid input. Please try again.\n");
                continue;
            }
            break;
        }

        if(JoinGame(&info,port,ip) != 0)
        {
            perror("Failed to join game");           
            return EXIT_FAILURE;
        }        
        Run(&info);
        break;
    }        
    case 3:
        printf("Ending...\n");
        break;
    }    
	
    close(info.sockfd);
    RemoveGame(&info.game);
    return 0;
}
