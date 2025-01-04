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
bool CheckHeadCollision(GameInfo* game, PlayerArrayInfo* player);
int AddPlayer(GameInfo* game);
void MovePlayer(GameInfo* game, Player* player);
int RemovePlayer(GameInfo* game, PlayerArrayInfo* player);
void GenerateApple(GameInfo* game);
void RemoveGame(GameInfo* game);


