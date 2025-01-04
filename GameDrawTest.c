#include "gameClient.h"
#include <time.h>

int main(int argc, char* argv)
{
    srand(time(NULL));
    GameInfo game;
    CreateGame(&game, 1, 30, 15, 0);
    AddPlayer(&game);
    Player * player = &game.players[0].player;
    Coord next = player->head;
    for(int i = 0; i < 18; ++i)
    {
        next.x++;
        if(next.x >= game.width)
        {
            next.x = 0;
        }
        AddPart(player, next);
    }
    GenerateApple(&game);
    
    DrawGame(&game);

    return 0;
}