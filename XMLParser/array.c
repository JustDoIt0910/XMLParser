#include "array.h"

Array* NewArray()
{
    Array* arr = (Array*)malloc(sizeof(Array));
    arr->capacity = INITIAL_CAP;
    arr->size = 0;
    arr->pData = (char**)malloc(INITIAL_CAP * sizeof(char*));
    return arr;
}

void FreeArray(Array* arr)
{
    for (int i = 0; i < arr->size; i++)
    {
        free(arr->pData[i]);
        arr->pData[i] = NULL;
    }
    free(arr->pData);
    arr->pData = NULL;
    free(arr);
    arr = NULL;
}

int isFull(Array* arr)
{
    return (arr->size == arr->capacity);
}

int Append(Array* arr, char* str)
{
    if (arr == NULL || str == NULL)
        return 0;
    if (isFull(arr)) //À©ÈÝ
    {
        char** new_addr = (char**)realloc(arr->pData, (arr->capacity + INCREMENT_CAP) * sizeof(char*));
        if (new_addr == NULL)
            return 0;
        if (new_addr != arr->pData)
            arr->pData = NULL;
        arr->pData = new_addr;
        arr->capacity += INCREMENT_CAP;
    }
    arr->pData[arr->size] = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(arr->pData[arr->size], str);
    arr->size += 1;
    return 1;
}

int Extend(Array* arr, Array* arr2)
{
    if (arr == NULL || arr2 == NULL)
        return 0;
    if (arr->size + arr2->size > arr->capacity) //À©ÈÝ
    {
        char** new_addr = (char**)realloc(arr->pData, (arr->size + arr2->size) * sizeof(char*));
        if (new_addr == NULL)
            return 0;
        if (new_addr != arr->pData)
            arr->pData = NULL;
        arr->pData = new_addr;
        arr->capacity = arr->size + arr2->size;
    }
    for (int i = 0; i < arr2->size; i++)
    {
        char* str = arr2->pData[i];
        arr->pData[arr->size + i] = (char*)malloc((strlen(str) + 1) * sizeof(char));
        strcpy(arr->pData[arr->size + i], str);
    }
    arr->size += arr2->size;
    return 1;
}

const char* Get(Array* arr, int index)
{
    if (index >= arr->size || index < 0)
        return NULL;
    return arr->pData[index];
}

void print(Array* arr) //debug
{
    printf("capacity: %d\n", arr->capacity);
    printf("length: %d\n", arr->size);
    for (int i = 0; i < arr->size; i++)
        printf("%s\n", arr->pData[i]);
}