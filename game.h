#include "player.h"
#include <stdbool.h>

typedef struct PlayerArrayInfo
{
    Player player;
    int index;
}PlayerArrayInfo;


typedef struct GameInfo
{
    int numOfPlayers, width, height;
    bool timeEnd;
    int gameTime;
    PlayerArrayInfo* players;
    int numOfCurPLayers;
    List apples;
}GameInfo;

void CreateGame(GameInfo* game, int numOfplayers, int width, int height, int gameTime);
int AddPlayer(GameInfo* game);
bool GameCheckCollisionWithPlayers(GameInfo* game, PlayerArrayInfo* player);
void MovePlayer(GameInfo* game, PlayerArrayInfo* player);
int RemovePlayer(GameInfo* game, PlayerArrayInfo* player);
void GenerateApple(GameInfo* game);
void RemoveGame(GameInfo* game);

bool ContainsPlayerHead(GameInfo* game,int y, int x, int index);
bool ContainsPlayerBody(GameInfo* game,int y, int x);
bool ContainsApple(GameInfo* game,int y, int x, int * index);//index on which the apple is in List
void DrawGame(GameInfo* game);


