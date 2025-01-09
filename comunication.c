#include "comunication.h"

void SerializeInitMessage(char* buffer, GameInfo* game)
{
    if (game == NULL) 
    {
        fprintf(stderr, "Error: GameInfo is NULL\n");
        return;
    }
    char * ptr = buffer;

    //numOfPlayers
    memcpy(ptr, &game->numOfPlayers, sizeof(int));
    ptr += sizeof(int);

    //width
    memcpy(ptr, &game->width, sizeof(int));
    ptr += sizeof(int);

    //height
    memcpy(ptr, &game->height, sizeof(int));
    ptr += sizeof(int);

    //numOfWalls
    memcpy(ptr, &game->numOfWalls, sizeof(int));  
    ptr += sizeof(int);

    //walls    
    if(game->containsWalls)
    {
        memcpy(ptr, game->walls, sizeof(Coord) * game->numOfWalls);
    }
    
}
void DeserializeInitMessage(char* buffer, GameInfo* game)
{
    char* ptr = buffer;
    int numOfPlayers, width, height, numOfWalls;
    //numOfPlayers    
    memcpy(&numOfPlayers, ptr, sizeof(int));
    ptr += sizeof(int);
    //width
    memcpy(&width, ptr, sizeof(int));
    ptr += sizeof(int);

    //height
    memcpy(&height, ptr, sizeof(int));
    ptr += sizeof(int);

    //numOfWalls
    memcpy(&numOfWalls, ptr, sizeof(int));   
    ptr += sizeof(int);
    
    CreateGame(game, numOfPlayers, width, height, 0, false); 
    game->containsWalls = numOfWalls > 0;
    game->numOfWalls = numOfWalls;

    //walls
    if(game->containsWalls)
    {   
        game->walls = malloc(sizeof(Coord) * numOfWalls);
        if (game->walls == NULL) 
        {
            printf("Error: Memory allocation for walls failed\n");
            return;
        }

        memcpy(game->walls, ptr, sizeof(Coord) * numOfWalls);
    }    
}

size_t SerializeServerMessage(char** buffer, GameInfo* game)
{
    if (game == NULL) 
    {
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
    size += appleListSize + sizeof(int) * 6 + sizeof(time_t);

    *buffer = malloc(size);
    if (*buffer == NULL) 
    {
        perror("malloc failed");
        return 0;
    }
    char* ptr = *buffer;
    
    memcpy(ptr, &game->runningTime, sizeof(time_t));
    ptr += sizeof(time_t);

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
    int numOfcur = game->numOfCurPLayers;
    memcpy(&game->runningTime, ptr, sizeof(time_t));
    ptr += sizeof(time_t);
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
        if(game->players[i].player.bodyParts.elements != NULL)
        {
            free(game->players[i].player.bodyParts.elements);
            game->players[i].player.bodyParts.elements = NULL;
        }
        game->players[i].player.bodyParts.elements = malloc(listSize);
        if(game->players[i].player.bodyParts.elements == NULL)
        {
            printf("Fockin NULL you minecraft picture\n");
            exit(EXIT_FAILURE);
        }
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
    if(game->apples.elements != NULL)
    {
        free(game->apples.elements);
        game->apples.elements = NULL;
    }
    game->apples.elements = malloc(listSize);
    memcpy(game->apples.elements, ptr, listSize);
    ptr += listSize;

    //playerIndex
    memcpy(playerIndex, ptr, sizeof(int));
}