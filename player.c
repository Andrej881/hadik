#include "player.h"
void CreatePlayer(Player * player, Coord coord)
{
    player->head = coord;

    player->capacity = 10;
    player->bodyParts = malloc(player->capacity * sizeof(Coord));
    player->numOfParts = 0;

    player->curDir = RIGHT;
}

void AddPart(Player * player, Coord coord)
{   
    ++player->numOfParts;
    if (player->numOfParts > player->capacity)
    {
        player->capacity *= 2;
        Coord* temp = realloc(player->bodyParts, player->capacity * sizeof(Coord));
        if (temp != NULL) 
        {
            player->bodyParts = temp;
        }
        else 
        {        
            fprintf(stderr, "Chyba: Nedostatok pamäte\n");
            exit(EXIT_FAILURE);
        }
    }   
    player->bodyParts[player->numOfParts-1] = coord;
}

void Move(Player* player)
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
    for (int i = 0; i < player->numOfParts; ++i)
    {
        Coord partCoordBeforeMove2 = player->bodyParts[i];
        player->bodyParts[i] = partCoordBeforeMove;
        partCoordBeforeMove = partCoordBeforeMove2;
    }
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
    free(player->bodyParts);
    player->bodyParts = NULL;
}