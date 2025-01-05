#include "player.h"
void CreatePlayer(Player * player, Coord coord)
{
    player->head = coord;

    CreatList(&player->bodyParts, 15, sizeof(Coord));
    player->curDir = RIGHT;
}

void AddPart(Player * player, Coord coord)
{   
    Coord new = coord;
    printf("%d\n",new.x);
    AddList(&player->bodyParts, &new);
}

Coord Move(Player* player)
{    
    Coord partCoordBeforeMove = player->head;
    switch (player->curDir)
    {
    case UP:
        player->head.y--;  
        break;
    case DOWN:
        player->head.y++;
        break;
    case RIGHT:
        player->head.x++;
        break;
    case LEFT:
        player->head.x--;
        break;
    }          
    for (int i = 0; i < player->bodyParts.end; ++i)
    {
        Coord partCoordBeforeMove2 = *(Coord *)GetList(&player->bodyParts,i);
        *(Coord *)GetList(&player->bodyParts,i) = partCoordBeforeMove;
        partCoordBeforeMove = partCoordBeforeMove2;
    }
    return partCoordBeforeMove;
}

int TryChangeDir(Player* player, Direction dir)
{
    Direction forbidenDir;
    switch (dir)
    {
    case UP:
        forbidenDir = DOWN;           
        break;
    case DOWN:
        forbidenDir = UP; 
        break;
    case RIGHT:
        forbidenDir = LEFT; 
        break;
    case LEFT:
        forbidenDir = RIGHT; 
        break;
    } 
    if (player->curDir == forbidenDir)
    {
        return 1;
    }  
    player->curDir = dir;
    return 0;
}

void DeletePlayer(Player* player)
{
    FreeList(&player->bodyParts);
}