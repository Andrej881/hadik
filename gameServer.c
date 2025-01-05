#include "gameServer.h"

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
    game->numOfCurPLayers++;
    if(game->numOfCurPLayers > game->numOfPlayers)
    {
        printf("to much players");
        return -1;
    }
    int freeIndex = game->numOfCurPLayers-1;
    game->players[freeIndex].index = freeIndex;
    bool findingSpace = true;
    while(findingSpace)
    {
        game->players[freeIndex].player.head.x = rand() % game->width;
        game->players[freeIndex].player.head.y = rand() % game->height;
        if(!CheckHeadCollision(game,&game->players[freeIndex]))
        {
            findingSpace = false;
        }
    }
    CreatePlayer(&game->players[freeIndex].player, game->players[freeIndex].player.head);
    GenerateApple(game);
    return freeIndex;
}

bool CheckHeadCollision(GameInfo* game, PlayerArrayInfo* player)
{   
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        if(player->index == i)
        {
            continue;
        }
        if(player->player.head.x == game->players[i].player.head.x && player->player.head.y == game->players[i].player.head.y)
        {
            return true;
        }
    }
    return false;
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
    if (CheckHeadCollision(game, player))
    {
        return true;
    }
    if(true)
    {}
    if(true)
    {}
}

void MovePlayer(GameInfo* game, PlayerArrayInfo* player)
{    
    Move(player);   

    player->player.head.x = player->player.head.x < 0 ? game->width-1 : player->player.head.x;    
    player->player.head.y = player->player.head.y < 0 ? game->height-1 : player->player.head.y;
    player->player.head.x = player->player.head.x >= game->width ? 0 : player->player.head.x;   
    player->player.head.y = player->player.head.y >= game->height ? 0 : player->player.head.y;

    GameCheckCollisionWithPlayers(game, player);
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