#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INITIAL_CAP 10
#define INCREMENT_CAP 3

typedef struct array
{
    int capacity;
    int size;
    char** pData;
}Array;

Array* NewArray();
void FreeArray(Array* arr);
int isFull(Array* arr);
int Append(Array* arr, char* str);
int Extend(Array* arr, Array* arr2);
const char* Get(Array* arr, int index);
void print(Array* arr); //debug

#endif 
