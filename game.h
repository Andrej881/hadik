#include "player.h"
#include <time.h>

#define GAME_END_TIME 10

typedef struct PlayerArrayInfo
{
    Player player;
    int index;
}PlayerArrayInfo;


typedef struct GameInfo
{
    int numOfPlayers, width, height;
    time_t runningTime, zeroPlayersTimer;
    bool timeEnd;
    int gameDuration;
    bool containsWalls;
    PlayerArrayInfo* players;
    int numOfCurPLayers;
    List apples;
    Coord * walls;
}GameInfo;

void CreateGame(GameInfo* game, int numOfplayers, int width, int height, int gameDuration);
int AddPlayer(GameInfo* game);
bool GameCheckCollisionWithPlayers(GameInfo* game, PlayerArrayInfo* player);
void MovePlayer(GameInfo* game, PlayerArrayInfo* player);
int RemovePlayer(GameInfo* game, PlayerArrayInfo* player);
void GenerateApple(GameInfo* game);
void RemoveGame(GameInfo* game);
void ResetPlayerInGame(GameInfo* game, int index);

int ContainsPlayerHead(GameInfo* game,int y, int x, int index);// return 0-false 1-true -1-head of player on index
int ContainsPlayerBody(GameInfo* game,int y, int x, int index);
bool ContainsApple(GameInfo* game,int y, int x, int * index);//index on which the apple is in List
void DrawGame(GameInfo* game, int playerIndex);//-1 means all players look the same

void PrintGameContent(GameInfo* game);


