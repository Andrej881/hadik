#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct List
{
    int capacity, start, end;
    int elementSize;
    void* elements;
} List;

void CreatList(List* list, int capacity, int elementSize);
void ChangeCapacityList(List* list, int capacity);
int AddList(List* list, void* element);
void* GetList(List* list, int index);
void* RemoveList(List* list, int index);
void FreeList(List* list);