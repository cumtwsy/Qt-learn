#ifndef DATACOLLECT_H
#define DATACOLLECT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char* diandaolv;
    char* ph;
    char* WT;
    char* rongjieyang;
    char* zhuodu;
    char* andan;
    char* battery;
    char* air_temp;
} DATA3;

void test_collect(void);

char* delete_tail(char* data);


#endif // DATACOLLECT_H

