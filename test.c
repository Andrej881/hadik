#include "game.h"

int main()
{
    GameInfo game;
    for (int i = 5;i < 25; i += 5)
    {
        CreateGame(&game, 2, 7, 7, 0, i);
        printf("\n");
        RemoveGame(&game);
    }
    return 0;
}