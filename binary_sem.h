#ifndef BINARY_SEMS_H
#define BINARY_SEMS_H

#include <errno.h>

extern int bsUseSemUndo;
extern int bsRetryonEintr;

//sets semaphore value to 1
int initSemAvailable(int semId, int semNum);
//sets semaphore value to 0
int initSemInUse(int semId, int semNum);
//adds to value
int reserveSem(int semId, int semNum);
//subtracts from value
int releaseSem(int semId, int semNum); 

#endif