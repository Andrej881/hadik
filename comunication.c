#include "comunication.h"

size_t SerializeServerMessage(char** buffer, GameInfo* game)
{
    size_t size, appleListSize;
    size_t playerListSize[game->numOfCurPLayers];
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        playerListSize[i] = game->players[i].player.bodyParts.capacity * game->players[i].player.bodyParts.elementSize;
        size += playerListSize[i] + sizeof(int) * (4 + 2);
    }
    appleListSize = game->apples.capacity * game->apples.elementSize;
    size += appleListSize + sizeof(int) * 4;

    *buffer = malloc(size);
    char* ptr = *buffer;

    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {    
        //Head
        memcpy(ptr, &game->players[i].player.head, sizeof(Coord));
        ptr += sizeof(Coord);
        //List metadata
        memcpy(ptr, &game->players[i].player.bodyParts.capacity, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &game->players[i].player.bodyParts.start, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &game->players[i].player.bodyParts.end, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &game->players[i].player.bodyParts.elementSize, sizeof(int));
        ptr += sizeof(int);

        //List elements
        memcpy(ptr, game->players[i].player.bodyParts.elements, playerListSize[i]);
    }    

    //Apples
    //List metadata
    memcpy(ptr, &game->apples.capacity, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, &game->apples.start, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, &game->apples.end, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, &game->apples.elementSize, sizeof(int));
    ptr += sizeof(int);

    //List elements
    memcpy(ptr, game->apples.elements, appleListSize);

    return size;
}

void DeserializeServerMessage(char* buffer, GameInfo* game)
{
    char* ptr = buffer;

    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        //Head   
        memcpy(&game->players[i].player.head, ptr, sizeof(Coord));
        ptr += sizeof(struct Coord);

        //List metadata
        memcpy(&game->players[i].player.bodyParts.capacity, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&game->players[i].player.bodyParts.start, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&game->players[i].player.bodyParts.end, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&game->players[i].player.bodyParts.elementSize, ptr, sizeof(int));
        ptr += sizeof(int);

        // Allocate memory for List elements and deserialize
        size_t listSize = game->players[i].player.bodyParts.capacity * game->players[i].player.bodyParts.elementSize;
        game->players[i].player.bodyParts.elements = malloc(listSize);
        memcpy(game->players[i].player.bodyParts.elements, ptr, listSize);
    }

    //Apples
    //List metadata
    memcpy(&game->apples.capacity, ptr, sizeof(int));
    ptr += sizeof(int);
    memcpy(&game->apples.start, ptr, sizeof(int));
    ptr += sizeof(int);
    memcpy(&game->apples.end, ptr, sizeof(int));
    ptr += sizeof(int);
    memcpy(&game->apples.elementSize, ptr, sizeof(int));
    ptr += sizeof(int);

    //List elements
    size_t listSize = game->apples.capacity * game->apples.elementSize;
    game->apples.elements = malloc(listSize);
    memcpy(game->apples.elements, ptr, listSize);
}