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
    
    //gameDuration
    memcpy(ptr, &game->gameDuration, sizeof(int));
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
    
    int numOfPlayers, width, height, numOfWalls, gameDuration;
    //numOfPlayers    
    memcpy(&numOfPlayers, ptr, sizeof(int));
    ptr += sizeof(int);
    //width
    memcpy(&width, ptr, sizeof(int));
    ptr += sizeof(int);

    //height
    memcpy(&height, ptr, sizeof(int));
    ptr += sizeof(int);

    //gameDuration
    memcpy(&gameDuration, ptr, sizeof(int));
    ptr += sizeof(int);

    //numOfWalls
    memcpy(&numOfWalls, ptr, sizeof(int));   
    ptr += sizeof(int);
    
    CreateGame(game, numOfPlayers, width, height, gameDuration, 0); 
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
        size += playerListSize[i] + (sizeof(int) * (4 + 3)) + sizeof(bool);
    }
    appleListSize = game->apples.capacity * game->apples.elementSize;
    size += appleListSize + sizeof(int) * 6 + sizeof(time_t) + 1;

    *buffer = calloc(size,1);
    if (*buffer == NULL) 
    {
        perror("malloc failed");
        return 0;
    }
    char* ptr = *buffer;    
    ptr[0] = 'R';
    ptr++;
    memcpy(ptr, &game->runningTime, sizeof(time_t));
    ptr += sizeof(time_t);

    //numOfCurPlayers
    memcpy(ptr, &game->numOfCurPLayers, sizeof(int));
    ptr += sizeof(int);
    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {    
        //maxScore
        memcpy(ptr, &game->players[i].player.maxScore, sizeof(int));
        ptr += sizeof(int);
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
    ptr++;  
    int numOfcur = game->numOfCurPLayers;
    memcpy(&game->runningTime, ptr, sizeof(time_t));
    ptr += sizeof(time_t);
    //curPlayers
    memcpy(&game->numOfCurPLayers, ptr, sizeof(int));
    ptr += sizeof(int);

    for (int i = 0; i < game->numOfCurPLayers; ++i)
    {      
        int maxScore;
        Coord head;
        bool dead;
        //maxScore
        memcpy(&maxScore, ptr, sizeof(int));
        ptr += sizeof(int);  
        //Head   
        memcpy(&head, ptr, sizeof(Coord));
        ptr += sizeof(Coord);
        //Dead
        memcpy(&dead, ptr, sizeof(bool));
        ptr += sizeof(bool);

        CreatePlayer(&game->players[i].player, head);
        game->players[i].player.maxScore = maxScore;
        game->players[i].player.dead = dead;

        int cap, start, end, elSize;
        //List metadata
        memcpy(&cap, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&start, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&end, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&elSize, ptr, sizeof(int));
        ptr += sizeof(int);

        FreeList(&game->players[i].player.bodyParts);
        CreatList(&game->players[i].player.bodyParts, cap, elSize);
        game->players[i].player.bodyParts.start = start;
        game->players[i].player.bodyParts.end = end;
        // Allocate memory for List elements and deserialize
        size_t listSize = game->players[i].player.bodyParts.capacity * game->players[i].player.bodyParts.elementSize;        
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

    game->players[*playerIndex].index = *playerIndex;
}