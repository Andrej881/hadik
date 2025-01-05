#include <stdio.h>
#include <stdlib.h>
#include "list.h"

typedef enum {UP, DOWN, RIGHT, LEFT} Direction;

typedef struct Coord{
    int x,y;
} Coord;

typedef struct Player{
    Coord head;
    List bodyParts;
    Direction curDir;
} Player;

void CreatePlayer(Player *  player, Coord coord);
void AddPart(Player * player, Coord coord);
Coord Move(Player* player);
int TryChangeDir(Player* player, Direction dir);
void DeletePlayer(Player* player);