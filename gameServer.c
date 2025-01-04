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

void MovePlayer(GameInfo* game, Player* player)
{    
    Move(player);

    player->head.x = player->head.x < 0 ? game->width-1 : player->head.x;    
    player->head.y = player->head.y < 0 ? game->height-1 : player->head.y;
    player->head.x = player->head.x >= game->width ? 0 : player->head.x;   
    player->head.y = player->head.y >= game->height ? 0 : player->head.y;
}