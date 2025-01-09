#include "player.h"
void CreatePlayer(Player * player, Coord coord)
{
    player->head = coord;
    player->dead = false;
    CreatList(&player->bodyParts, 15, sizeof(Coord));
    player->curDir = RIGHT;
}

void AddPart(Player * player, Coord coord)
{   
    Coord new = coord;
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
    if(player->bodyParts.end == 0)
        return 0;
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

void PrintPlayer(Player* player)
{
    printf("Head [%d, %d]\n", player->head.x, player->head.y);
    printf("BodyParts: ");
    for (int i = 0; i < player->bodyParts.end; ++i)
    {
        Coord coord = *(Coord *)GetList(&player->bodyParts, i);
        printf(" [%d, %d]", coord.x, coord.y);
    }
    printf("\n");
}

void ResetPlayer(Player * player, Coord coord)
{
    DeletePlayer(player);
    CreatePlayer(player, coord);
}