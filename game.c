#include "game.h"

void CreateGame(GameInfo* game, int numOfplayers, int width, int height, int gameDuration,int numOfWalls)
{    
    game->running = true;
    game->numOfAddedTraps = 0;
    game->numOfPlayers = numOfplayers;
    game->width = width;
    game->height = height;
    game->runningTime = 0;
    game->containsWalls = numOfWalls > 0;
    game->gameDuration = gameDuration;
    if(gameDuration > 0)
    {
        game->timeEnd = true;
    }  
    else
    {
        game->timeEnd = false;
    }  

    game->players = malloc(numOfplayers * sizeof(PlayerArrayInfo));  
    for (int i = 0; i < numOfplayers; ++i)
    {
        game->players[i].index = -1;
    }
    CreatList(&game->apples, 10, sizeof(Coord));        
    game->numOfCurPLayers = 0;
    if(numOfWalls > 0)
    {     
        game->numOfWalls = numOfWalls;
        GenerateWalls(game);
    }
    else
    {
        game->numOfWalls = 0;
        game->walls = NULL;
    }
}

int CreateGameFromFile(GameInfo* game, const char* path)
{   
    printf("loading from file: %s\n", path);
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    int numOfPlayers, width, height, gameDuration, numOfWalls;
    if (fscanf(file, "%d %d %d %d %d",
               &numOfPlayers,
               &width,
               &height,
               &gameDuration,
               &numOfWalls) != 5) {
        printf("Error reading game info\n");
        fclose(file);
        return -2;
    }

    CreateGame(game, numOfPlayers, width, height, gameDuration, false);
    game->numOfWalls = numOfWalls;
    if (game->numOfWalls > 0) {
        game->containsWalls = true;
        game->walls = malloc(sizeof(Coord) * game->numOfWalls);
        if (game->walls == NULL) {
            perror("Error allocating memory for walls");
            fclose(file);
            return -3;
        }

        for (int i = 0; i < game->numOfWalls; ++i) {
            if (fscanf(file, "%d %d", &game->walls[i].x, &game->walls[i].y) != 2) {
                printf("Error reading wall coordinates[%d] [%d]\n", game->walls[i].x, game->walls[i].y);
                free(game->walls);
                fclose(file);
                return -4;
            }
        }
    } else {
        game->walls = NULL;
    }
    fclose(file);
    return 0;
}

int SaveGameSetUp(GameInfo* game, const char* filePath)
{
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    if(fprintf(file, "%d %d %d %d %d ",
    game->numOfPlayers,
    game->width,
    game->height,
    game->gameDuration,
    game->containsWalls ? game->numOfWalls : 0) < 5)
    {
        printf("Error writing game info\n");
        fclose(file);
        return -1;
    } 

    if (game->numOfWalls > 0) {

        for (int i = 0; i < game->numOfWalls; ++i) {
            if (fprintf(file, "%d %d ", game->walls[i].x, game->walls[i].y) < 2) {
                printf("Error writing wall coordinates\n");
                fclose(file);
                return -2;
            }
        }
    } 
    fclose(file);
    return 0;
}

void GenerateWalls(GameInfo* game)
{       
    game->walls = malloc(game->numOfWalls * sizeof(Coord));
    int map[game->width][game->height];
    memset(map, 0, sizeof(map));

    for (int i = 0; i < game->numOfWalls; ++i)
    {
        game->walls[i].x = -1;
        game->walls[i].y = -1;
    }
    Coord badCoords[game->width * game->height];
    int numOfBadCoords = 0;
    for (int i = 0; i < game->numOfWalls; ++i)
    {
        Coord coord;
        while(1)
        {
            coord.x = rand() % game->width;
            coord.y = rand() % game->height;
            if(!ContainsWall(game, coord.y, coord.x) && IsConnected(game, coord.y, coord.x,map) && !CreatesDeadEnd(game, coord.x, coord.y, map))
                break;
            else
            {           
                bool add = true;     
                for (int i = 0; i < numOfBadCoords; ++i)
                {
                    if(badCoords[i].x == coord.x && badCoords[i].y == coord.y)
                        add = false;
                }
                if(add)
                    badCoords[numOfBadCoords++] = coord;
                if(numOfBadCoords >= game->width * game->height)
                {
                    printf("Cannot generate walls without making it unplayable Current walls:\n");
                    for(int i = 0; i < game->height; ++i)
                    {
                        for (int j = 0; j < game->width; ++j)
                        {
                            printf("%d ",map[j][i]);
                        }
                        printf("\n");
                    }
                    return;
                }
            }
        }
        game->numOfAddedTraps++;
        game->walls[i] = coord;
        map[coord.x][coord.y] = 1;
    }
    for(int i = 0; i < game->height; ++i)
    {
        for(int j = 0; j < game->width; ++j)
        {
            printf("%d ",map[j][i]);
        }
        printf("\n");
    }
    printf("\n");
}

bool IsConnected(GameInfo* game,int cY, int cX, int map[game->width][game->height])
{
    int visited[game->width][game->height];    
    memset(visited, 0, sizeof(visited));
    int numOfVisitedPlaces = 0;
    int BFSbuffer[(game->height * game->width)][2];
    int start = 0, end = 0;

    int startX = -1, startY = -1;
    for(int i = 0; i < game->height; ++i)
    {
        for(int j = 0; j < game->width; ++j)
        {
            if(map[j][i] == 0)
            {
                startX = j;
                startY = i;
                numOfVisitedPlaces++;
                break;
            }
        }
        if(startX != -1)
            break;
    }

    visited[startX][startY] = 1;
    BFSbuffer[end][0] = startX;
    BFSbuffer[end++][1] = startY;

    Coord dir[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while(start < end)
    {
        int x = BFSbuffer[start][0];
        int y = BFSbuffer[start++][1];
        for(int i = 0; i < 4; ++i)
        {
            Coord nextDir = dir[i];
            int nextX = x + nextDir.x;
            int nextY = y + nextDir.y;
            if(map[nextX][nextY] == 0 && visited[nextX][nextY] == 0 && nextX >= 0 && nextX < game->width && nextY >= 0 && nextY < game->height && (nextX != cX || nextY != cY))
            {                            
                BFSbuffer[end][0] = nextX;
                BFSbuffer[end++][1] = nextY;
                visited[nextX][nextY] = 1;
                numOfVisitedPlaces++;
            }
        }
    }    

    return numOfVisitedPlaces == (game->width * game->height) - game->numOfAddedTraps - 1;
}

bool CreatesDeadEnd(GameInfo* game, int x, int y, int map[game->width][game->height])
{
    Coord directions[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};    

    for (int i = 0; i < 4; ++i)
    {
        int placeX = x + directions[i].x;
        int placeY = y + directions[i].y;
        
        int freeNeighbors = 0;
        for (int j = 0; j < 4; ++j)
        {  
            int nextX = placeX + directions[j].x;
            int nextY = placeY + directions[j].y;
            if(nextX == x && nextY == y)
                continue;
            if (nextX >= 0 && nextX < game->width && nextY >= 0 && nextY < game->height && map[nextX][nextY] == 0)
            {
                freeNeighbors++;
            }
        }
        if(freeNeighbors < 2)
            return true;
    }

    return false; 
}

bool ContainsWall(GameInfo* game,int y, int x)
{
    for (int i = 0; i < game->numOfWalls; ++i)
    {
        if(game->walls[i].x == x && game->walls[i].y == y)
        {
            return true;
        }
    }
    return false;
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
        findingSpace = false;
        head.x = rand() % game->width;
        head.y = rand() % game->height;
        if(ContainsPlayerHead(game, head.x, head.y, -1) != 0)
        {
            findingSpace = true;
        }
        else if(ContainsPlayerBody(game, head.x, head.y, -1) != 0)
        {
            findingSpace = true;
        }
        else if(game->containsWalls && ContainsWall(game, head.x, head.y))
        {
            findingSpace = true;
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
    if (game->containsWalls && ContainsWall(game, head.y, head.x))
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
    if(player->player.head.x < 0)
    {
        if(game->containsWalls)
            player->player.dead = true;
        else        
            player->player.head.x = game->width - 1;
    }        
    if(player->player.head.y < 0)
    {
        if(game->containsWalls)
            player->player.dead = true;
        else        
            player->player.head.y = game->height - 1;
    } 
    if(player->player.head.x >= game->width)
    {
        if(game->containsWalls)
            player->player.dead = true;
        else        
            player->player.head.x = 0;
    }     
    if(player->player.head.y >= game->height)
    {
        if(game->containsWalls)
            player->player.dead = true;  
        else        
            player->player.head.y = 0;
    } 

    if (GameCheckCollisionWithPlayers(game, player))
    {
        player->player.dead = true;    
    }
    int appleIndex;
    if (ContainsApple(game, player->player.head.y, player->player.head.x, &appleIndex))
    {
        AddPart(&player->player, coord);
        free(RemoveList(&game->apples, appleIndex));
        if(game->apples.end < game->numOfCurPLayers)
            GenerateApple(game);
    }   
}

void GenerateApple(GameInfo* game)
{
    Coord apple;
    while(1)
    {
        apple.x = rand() % game->width;        
        apple.y = rand() % game->height;
        if(ContainsPlayerHead(game, apple.y, apple.x, -1) || ContainsPlayerBody(game, apple.y, apple.x, -1) || ContainsApple(game, apple.y, apple.x, NULL) || (game->containsWalls && ContainsWall(game, apple.y, apple.x)))
        {
            continue;
        }
        break;
    }

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
void AddTime(GameInfo* game ,time_t time)
{
    game->runningTime += time;
    if(game->timeEnd && game->runningTime >= game->gameDuration)
        game->running = false;
}
void DrawGame(GameInfo* game, int playerIndex)
{          
    system("clear");
    if (playerIndex >= 0)
        printf("Score: %d\tTime %ld\n", game->players[playerIndex].player.bodyParts.end, game->runningTime);
    for(int i = -1; i <= game->height; ++i)
    {        
        if(i == -1 || i == game->height)
        {
            for (int j = -1; j <= game->width; ++j)
            {
                printf("# ");
            }
            printf("\n");
            continue;
        }
        if(playerIndex >= 0 && game->players[playerIndex].player.dead && i == game->height / 2 - 2)
        {
            printf("\tYou died\n\tFinalScore [%d]\n\tPress Enter to play again\n\tPress q to leave\n", game->players[playerIndex].player.bodyParts.end);
            continue;
        }
        for(int j = -1; j <= game->width; ++j)
        {    
            if(j == -1 || j == game->width)
            {
                printf("# ");
                continue;
            }
            int headTest = ContainsPlayerHead(game, i, j, playerIndex);
            int bodyTest = ContainsPlayerBody(game, i, j, playerIndex);
            if(headTest == 1)
            {
                printf("Q ");
            }
            else if(headTest == -1)
            {
                printf("@ ");
            }
            else if(bodyTest == 1)
            {
                printf("U ");
            }
            else if(bodyTest == -1)
            {
                printf("O ");
            }
            else if(ContainsApple(game, i, j, NULL))
            {
                printf("* ");
            }
            else if(game->containsWalls && ContainsWall(game, i, j))
            {
                printf("# ");
            }
            else
            {
                printf("  ");
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
    for (int i = 0;i < game->numOfWalls; ++i)
    {
        printf("Wall-[%d %d]\n",game->walls[i].x, game->walls[i].y);        
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
void PrintLeaderBoard(GameInfo* game, int playerIndex)
{
    printf("Game has ended:\n");
    printf("LeaderBoard:\n");
    for (int i = 0; i < game->numOfPlayers; ++i)
    {
        if(game->players[i].index == playerIndex)
        {
            printf("You: \t");
        }
        else
        {
            printf("Player %d", i);
        }
        printf("[%d]\n", game->players[i].player.bodyParts.end);
    }
}