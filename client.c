#include <time.h> 

#include "clientGame.h"

#define SCREEN_WIDTH 30
#define SCREEN_HEIGHT 11

void setup_terminal(struct termios *original) {
    struct termios new_settings;
    tcgetattr(0, original);             
    new_settings = *original;
    new_settings.c_lflag &= ~(ICANON | ECHO); 
    new_settings.c_cc[VMIN] = 1;             
    new_settings.c_cc[VTIME] = 0;  
    tcsetattr(0, TCSANOW, &new_settings);    
}

void reset_terminal(struct termios *original) {
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

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

int GameMenu()
{
    int position = 2;
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
                    return position;
                    break;
                default:
                    reading = true;
                    break;                
                }
            } 
        }
        
    }    
}

int main(int argc, char *argv[]) {
    ClientGameInfo info;    

	struct termios original_settings;
    setup_terminal(&original_settings); 
    srand(time(NULL)); 

    if (argc < 2) {
        fprintf(stderr,"usage %s hostname\n", argv[0]);
        return EXIT_FAILURE;
    }    
	const char* ip = argv[1];
    //int port = atoi(argv[2]);
    
    switch(GameMenu())
    {        
    case 1:
        NewGame(&info);        
        setup_terminal(&original_settings); 
        break;
    case 2:
    {        
        reset_terminal(&original_settings);
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
                // Vyčistenie vstupného bufferu
                while (getchar() != '\n');
            }
            printf("Write Game Port: \n");
        }
        setup_terminal(&original_settings); 
        if(JoinGame(&info,port,ip) != 0)
        {
            perror("Failed to join game");
            reset_terminal(&original_settings);            
            return EXIT_FAILURE;
        }        
        break;
    }        
    case 3:
        printf("Ending...\n");
        break;
    }    
	
    close(info.sockfd);
    reset_terminal(&original_settings);
    return 0;
}
