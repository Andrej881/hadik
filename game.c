#include "game.h"

void GenerateWalls(GameInfo* game)
{       
    game->walls = malloc(((game->width * game->height)/4) * sizeof(Coord));
    int map[game->width][game->height];
    memset(map, 0, sizeof(map));

    for (int i = 0; i < (game->width * game->height)/4; ++i)
    {
        game->walls[i].x = -1;
        game->walls[i].y = -1;
    }
    for (int i = 0; i < (game->width * game->height)/4; ++i)
    {
        Coord coord;
        while(1)
        {
            coord.x = rand() % game->width;
            coord.y = rand() % game->height;
            if(!ContainsWall(game, coord.x, coord.y) && !IsConnected(game, coord.x, coord.y,map))
                break;
        }
        game->numOfAddedTraps++;
        game->walls[i] = coord;
        map[coord.y][coord.x] = 1;
    }
}

bool IsConnected(GameInfo* game,int y, int x, int map[game->width][game->height])
{
    int visited[game->width][game->height];    
    memset(visited, 0, sizeof(visited));
    int numOfVisitedPlaces = 0;
    int BFSbuffer[(game->height * game->width)][2];
    int start = 0, end = 0;

    int start_x = -1, start_y = -1;
    for(int i = 0; i < game->width; ++i)
    {
        for(int j = 0; j < game->height; ++j)
        {
            if(map[i][j] == 0)
            {
                start_x = i;
                start_y = j;
                numOfVisitedPlaces++;
                break;
            }
        }
        if(start_x != -1)
            break;
    }

    visited[start_x][start_y] = 1;
    BFSbuffer[end][0] = start_x;
    BFSbuffer[end++][1] = start_y;

    Coord dir[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while(start < end)
    {
        int x = BFSbuffer[start][0];
        int y = BFSbuffer[start++][1];
        for(int i = 0; i < 4; ++i)
        {
            Coord nextDir = dir[i];
            if(map[x + nextDir.x][y + nextDir.y] == 0 && nextDir.x >= 0 && nextDir.x < game->width && nextDir.y >= 0 && nextDir.y < game->height && visited[nextDir.x][nextDir.y] == 0)
            {                            
                BFSbuffer[end][0] = nextDir.x;
                BFSbuffer[end++][1] = nextDir.y;
                visited[nextDir.x][nextDir.y] = 1;
                numOfVisitedPlaces++;
            }
        }
    }

    return numOfVisitedPlaces == (game->width * game->height) - game->numOfAddedTraps;
}

bool ContainsWall(GameInfo* game,int y, int x)
{
    for (int i = 0; i < (game->width * game->height)/4; ++i)
    {
        if(game->walls[i].x = x && game->walls[i].y == y)
        {
            return true;
        }
    }
    return false;
}

void CreateGame(GameInfo* game, int numOfplayers, int width, int height, int gameDuration, bool walls)
{    
    game->numOfAddedTraps = 0;
    game->numOfPlayers = numOfplayers;
    game->width = width;
    game->height = height;
    game->runningTime = 0;
    game->containsWalls = walls;
    if(gameDuration > 0)
    {
        game->gameDuration = gameDuration;
        game->timeEnd = true;
    }  
    else
    {
        game->timeEnd = false;
    }  

    game->players = malloc(numOfplayers * sizeof(PlayerArrayInfo));  
    CreatList(&game->apples, 10, sizeof(Coord));        
    game->numOfCurPLayers = 0;
    if(walls)
    {
        GenerateWalls(game);
    }
}

int AddPlayer(GameInfo* game)
{    
    if(game->numOfCurPLayers >= game->numOfPlayers)
    {
        printf("to much players");
        return -1;
    }
    int freeIndex = game->numOfCurPLayers;
    game->players[freeIndex].index = freeIndex;
    bool findingSpace = true;
    Coord head;
    while(findingSpace)
    {
        head.x = rand() % game->width;
        head.y = rand() % game->height;
        if(ContainsPlayerHead(game, head.x, head.y, -1) == 0)
        {
            findingSpace = false;
        }
        if(ContainsPlayerBody(game, head.x, head.y, -1) == 0)
        {
            findingSpace = false;
        }
    }
    CreatePlayer(&game->players[freeIndex].player, head);
    game->numOfCurPLayers++;
    
    if(game->apples.end < game->numOfCurPLayers)
        GenerateApple(game);
    return freeIndex;
}

int RemovePlayer(GameInfo* game, PlayerArrayInfo* player)
{
    int index = player->index;
    DeletePlayer(&player->player);
    for (int i = index + 1; i < game->numOfCurPLayers; ++i)
    {
        game->players[i-1] = game->players[i];
        game->players[i-1].index = i-1;
    }

    game->numOfCurPLayers--;
    return index;
}

bool GameCheckCollisionWithPlayers(GameInfo* game, PlayerArrayInfo* player)
{
    Coord head = player->player.head;
    if (ContainsPlayerHead(game, head.y, head.x, player->index) == 1)
    {
        return true;
    }
    if (ContainsPlayerBody(game, head.y, head.x, -1) != 0)
    {
        return true;
    }
    return false;
}

void MovePlayer(GameInfo* game, PlayerArrayInfo* player)
{    
    if(player->player.dead)
        return;
    Coord coord = Move(&player->player);   

    player->player.head.x = player->player.head.x < 0 ? game->width-1 : player->player.head.x;    
    player->player.head.y = player->player.head.y < 0 ? game->height-1 : player->player.head.y;
    player->player.head.x = player->player.head.x >= game->width ? 0 : player->player.head.x;   
    player->player.head.y = player->player.head.y >= game->height ? 0 : player->player.head.y;

    if (GameCheckCollisionWithPlayers(game, player))
    {
        player->player.dead = true;
    }
    int appleIndex;
    if (ContainsApple(game, player->player.head.y, player->player.head.x, &appleIndex))
    {
        AddPart(&player->player, coord);
        RemoveList(&game->apples, appleIndex);
        if(game->apples.end < game->numOfCurPLayers)
            GenerateApple(game);
    }
}

void GenerateApple(GameInfo* game)
{
    Coord apple;
    apple.x = rand() % game->width;        
    apple.y = rand() % game->height;
    AddList(&game->apples, &apple);
}

void RemoveGame(GameInfo* game)
{
    FreeList(&game->apples);
    for(int i = game->numOfCurPLayers-1; i >= 0; i--)
    {
        RemovePlayer(game, &game->players[i]);
    }
    free(game->players);
    game->players = NULL;
    if(game->containsWalls)
    {
        free(game->walls);
        game->walls = NULL;
    }
}

void DrawGame(GameInfo* game, int playerIndex)
{          
    system("clear");
    if (playerIndex >= 0)
        printf("Score: %d\tTime %ld\n", game->players[playerIndex].player.bodyParts.end, game->runningTime);
    for(int i = 0; i < game->height; ++i)
    {        
        if(playerIndex >= 0 && game->players[playerIndex].player.dead && i == game->height / 2 - 3)
        {
            printf("\tYou died\n\tFinalScore [%d]\n\tPress Enter to play again\n\tPress q to leave\n", game->players[playerIndex].player.bodyParts.end);
            continue;
        }
        for(int j = 0; j < game->width; ++j)
        {    
            int headTest = ContainsPlayerHead(game, i, j, playerIndex);
            int bodyTest = ContainsPlayerBody(game, i, j, playerIndex);
            if(headTest == 1)
            {
                printf("Q");
            }
            else if(headTest == -1)
            {
                printf("@");
            }
            else if(bodyTest == 1)
            {
                printf("U");
            }
            else if(bodyTest == -1)
            {
                printf("O");
            }
            else if(ContainsApple(game, i, j, NULL))
            {
                printf("#");
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}

int ContainsPlayerHead(GameInfo* game,int y, int x, int index)//if index = -1 ignore 
{    
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {        
        Coord head = game->players[i].player.head;
        if(head.x == x && head.y == y && !game->players[i].player.dead)
        {           
            if(index == game->players[i].index)
            {
                return -1;
            } 
            else
            {
                return 1;
            }
        }
    }
    return 0;
}

int ContainsPlayerBody(GameInfo* game,int y, int x, int index)
{
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        if (game->players[i].player.dead)
            continue;
        List bodys = game->players[i].player.bodyParts;
        for(int j = 0; j < bodys.end; ++j)
        {
            Coord body = *(Coord *)GetList(&bodys, j);
            if(body.x == x && body.y == y)
            {
                if(index == game->players[i].index)
                {
                    return -1;
                } 
                else
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

bool ContainsApple(GameInfo* game,int y, int x, int * index)
{
    List apples = game->apples;
    for (int i = 0; i < apples.end; ++i)
    {
        Coord apple = *(Coord *)GetList(&apples, i);
        if(apple.x == x && apple.y == y)
        {
            if(index != NULL)
            {
                *index = i;
            }
            return true;
        }
    }
    return false;
}

void PrintGameContent(GameInfo* game)
{
    printf("width: %d height: %d players: %d/%d\nTime: %ld\n", game->width, game->height, game->numOfCurPLayers, game->numOfPlayers, game->runningTime);
    for (int i = 0; i < game->apples.end; ++i)
    {
        Coord coord = *(Coord *) GetList(&game->apples, i);
        printf("apple %d [%d, %d]\n",i, coord.x, coord.y);
    }
    for (int i = 0;i < game->numOfCurPLayers; ++i)
    {
        PrintPlayer(&game->players[i].player);        
    }
}

void ResetPlayerInGame(GameInfo* game, int index)
{
    bool findingSpace = true;
    Coord head;
    while(findingSpace)
    {
        head.x = rand() % game->width;
        head.y = rand() % game->height;
        if(ContainsPlayerHead(game, head.x, head.y, -1) == 0)
        {
            findingSpace = false;
        }
        if(ContainsPlayerBody(game, head.x, head.y, -1) == 0)
        {
            findingSpace = false;
        }
    }
    ResetPlayer(&game->players[index].player, head);
}