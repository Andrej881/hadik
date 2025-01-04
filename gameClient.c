#include "gameClient.h"

void DrawGame(GameInfo* game)
{
    system("clear");
    for(int i = 0; i < game->height; ++i)
    {        
        for(int j = 0; j < game->width; ++j)
        {
            if(ContainsPlayerHead(game,i,j))
            {
                printf("@");
            }
            else if(ContainsPlayerBody(game,i,j))
            {
                printf("O");
            }
            else if(ContainsApple(game,i,j))
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

bool ContainsPlayerHead(GameInfo* game,int y, int x)
{
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        Coord head = game->players->player.head;
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

bool ContainsApple(GameInfo* game,int y, int x)
{
    List apples = game->apples;
    for (int i = 0; i < apples.end; ++i)
    {
        Coord apple = *(Coord *)GetList(&apples, i);
        if(apple.x == x && apple.y == y)
        {
            return true;
        }
    }
    return false;
}