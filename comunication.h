#include "game.h"

void SerializeInitMessage(char* buffer, GameInfo* game);
void DeserializeInitMessage(char* buffer, GameInfo* game);

size_t SerializeServerMessage(char** buffer, GameInfo* game);//sending Player head and parts coords + Apple coords(later also walls) 
void DeserializeServerMessage(char* buffer, GameInfo* game, int * playerIndex);
