#include "game.h"

void CreateGame(GameInfo* game, int numOfplayers, int width, int height, int gameTime)
{
    game->numOfPlayers = numOfplayers;
    game->width = width;
    game->height = height;
    if(gameTime > 0)
    {
        game->gameTime = gameTime;
        game->timeEnd = true;
    }  
    else
    {
        game->timeEnd = false;
    }  

    game->players = malloc(numOfplayers * sizeof(PlayerArrayInfo));  
    CreatList(&game->apples, 10, sizeof(Coord));        
    game->numOfCurPLayers = 0;
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
        if(!ContainsPlayerHead(game, head.x, head.y, -1))
        {
            findingSpace = false;
        }
    }
    CreatePlayer(&game->players[freeIndex].player, head);
    game->numOfCurPLayers++;
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
    if (ContainsPlayerHead(game, head.y, head.x, player->index))
    {
        return true;
    }
    if(ContainsPlayerBody(game, head.y, head.x))
    {
        return true;
    }
    return false;
}

void MovePlayer(GameInfo* game, PlayerArrayInfo* player)
{    
    Coord coord = Move(&player->player);   

    player->player.head.x = player->player.head.x < 0 ? game->width-1 : player->player.head.x;    
    player->player.head.y = player->player.head.y < 0 ? game->height-1 : player->player.head.y;
    player->player.head.x = player->player.head.x >= game->width ? 0 : player->player.head.x;   
    player->player.head.y = player->player.head.y >= game->height ? 0 : player->player.head.y;

    if (GameCheckCollisionWithPlayers(game, player))
    {
        printf("LOST\n");
    }
    int appleIndex;
    if (ContainsApple(game, player->player.head.y, player->player.head.x, &appleIndex))
    {
        AddPart(&player->player, coord);
        RemoveList(&game->apples, appleIndex);
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
}

void DrawGame(GameInfo* game)
{
    system("clear");
    for(int i = 0; i < game->height; ++i)
    {        
        for(int j = 0; j < game->width; ++j)
        {
            if(ContainsPlayerHead(game,i,j,-1))
            {
                printf("@");
            }
            else if(ContainsPlayerBody(game,i,j))
            {
                printf("O");
            }
            else if(ContainsApple(game,i,j,NULL))
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

bool ContainsPlayerHead(GameInfo* game,int y, int x, int playerIndex)//if playerIndex = -1 ignore
{
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        if(playerIndex == game->players[i].index)
        {
            continue;
        }
        Coord head = game->players[i].player.head;
        if(head.x == x && head.y == y)
        {            
            return true;
        }
    }
    return false;
}

bool ContainsPlayerBody(GameInfo* game,int y, int x)
{
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        List bodys = game->players[i].player.bodyParts;
        for(int j = 0; j < bodys.end; ++j)
        {
            Coord body = *(Coord *)GetList(&bodys, j);
            if(body.x == x && body.y == y)
            {
                return true;
            }
        }
    }
    return false;
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