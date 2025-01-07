#include "comunication.h"

size_t SerializeServerMessage(char** buffer, GameInfo* game, int playerIndex)
{
    if (game == NULL) {
    fprintf(stderr, "Error: GameInfo is NULL\n");
    return 0;
    }
    size_t size = 0, appleListSize;
    size_t playerListSize[game->numOfCurPLayers];
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        playerListSize[i] = game->players[i].player.bodyParts.capacity * game->players[i].player.bodyParts.elementSize;
        size += playerListSize[i] + sizeof(int) * (4 + 2) + sizeof(bool);
    }
    appleListSize = game->apples.capacity * game->apples.elementSize;
    size += appleListSize + sizeof(int) * 6;

    *buffer = malloc(size);
    if (*buffer == NULL) {
    perror("malloc failed");
    return 0;
    }
    char* ptr = *buffer;
    //playerIndex
    memcpy(ptr, &playerIndex, sizeof(int));
    ptr += sizeof(int);

    //numOfCurPlayers
    memcpy(ptr, &game->numOfCurPLayers, sizeof(int));
    ptr += sizeof(int);
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {    
        //Head
        memcpy(ptr, &game->players[i].player.head, sizeof(Coord));
        ptr += sizeof(Coord);
        //Dead
        memcpy(ptr, &game->players[i].player.dead, sizeof(bool));
        ptr += sizeof(bool);

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
        ptr += playerListSize[i];
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

void DeserializeServerMessage(char* buffer, GameInfo* game, int* playerIndex)
{
    char* ptr = buffer;    
    
    //playerIndex
    memcpy(playerIndex, ptr, sizeof(int));
    ptr += sizeof(int);
    //curPlayers
    memcpy(&game->numOfCurPLayers, ptr, sizeof(int));
    ptr += sizeof(int);
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {
        //Head   
        memcpy(&game->players[i].player.head, ptr, sizeof(Coord));
        ptr += sizeof(Coord);
        //Dead
        memcpy(&game->players[i].player.dead, ptr, sizeof(bool));
        ptr += sizeof(bool);

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
        ptr += listSize;
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

    //printf("cap %d str %d end %d elSize %d\n",game->apples.capacity, game->apples.start, game->apples.end, game->apples.elementSize);

    //List elements
    size_t listSize = game->apples.capacity * game->apples.elementSize;
    game->apples.elements = malloc(listSize);
    memcpy(game->apples.elements, ptr, listSize);
}