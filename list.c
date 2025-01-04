#include "list.h"

void CreatList(List* list, int capacity, int elementSize)
{    
    list->capacity = capacity;
    list->elementSize = elementSize;
    list->start = list->end = 0;
    list->elements = malloc(capacity * elementSize);
    if (list->elements == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
}

void ChangeCapacityList(List* list, int capacity)
{
    void* temp = realloc(list->elements, capacity * list->elementSize);
    if (temp != NULL) 
    {
        list->elements = temp;
        list->capacity = capacity;
    }
    else 
    {        
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

int AddList(List* list, void* element)
{    
    if(list->end >= list->capacity)
    {
        ChangeCapacityList(list, list->capacity * 2);
    }
    void* target = (char*)list->elements + (list->end * list->elementSize);
    memcpy(target, element, list->elementSize);

    list->end++;
    return list->end;
}

void* RemoveList(List* list, int index)
{
    if (index < 0 || index >= list->end) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return NULL;
    }
    void* element = malloc(list->elementSize);
    if (element == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    void* source = (char*)list->elements + (index * list->elementSize);
    memcpy(element, source, list->elementSize);
    for (int i = index; i < list->end - 1; i++) {
        void* dest = (char*)list->elements + (i * list->elementSize);
        void* next = (char*)list->elements + ((i + 1) * list->elementSize);
        memcpy(dest, next, list->elementSize);
    }
    list->end--;
    return element;   
}

void* GetList(List* list, int index)
{
    if (index < 0 || index >= list->end) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return NULL;
    }
    return (char*)list->elements + (index * list->elementSize);
}

void FreeList(List* list)
{
    free(list->elements);
    list->elements = NULL;
    list->capacity = 0;
    list->end = 0;
}