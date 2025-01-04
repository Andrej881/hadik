#include <stdio.h>
#include <stdlib.h>

typedef enum {UP, DOWN, RIGHT, LEFT} Direction;

typedef struct Coord{
    int x,y;
} Coord;

typedef struct Player{
    Coord head;
    Coord* bodyParts;
    int numOfParts;
    int capacity;
    Direction curDir;
} Player;

void CreatePlayer(Player *  player, Coord coord);
void AddPart(Player * player, Coord coord);
void Move(Player* player);
int TryChangeDir(Player* player, Direction dir);
void DeletePlayer(Player* player);