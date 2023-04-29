// Jade Thompson, jet475
// Systems Programming, due 4/30/2023

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>

#include "semun.h"
#include "binary_sem.h"

#define OBJ_PERMS (S_IRUSR | S_IWUSR)

// shared memory segment -- stores number of pairs and the actual pairs
struct seg{
  int randBlocks;
  int blocks[20][2];
};

int checkError(int val, const char *msg){
  if (val == -1){
    perror(msg);
    exit(EXIT_FAILURE);
  }
  return val;
}

int childStuff(struct seg* smap, int semid, int shmid){
  int randLetter, randNum;

  // attach shared memory
  smap = shmat(shmid, NULL, 0);
  if (smap == (void *) -1) {
    checkError(-1, "shmat");
  }

  // seed rand()
  srand(time(NULL));

  checkError(reserveSem(semid, 0), "reserve child");

  // generate number of pairs/blocks
  smap->randBlocks = (rand() % 11) + 30;

  // generate pairs/blocks
  for (int i = 0; i < smap->randBlocks; i++){
    randLetter = (rand() % 26) + 97;
    randNum = (rand() % 9) + 2;
    smap->blocks[i][0] = randNum;
    smap->blocks[i][1] = randLetter;
  }
  
  checkError(releaseSem(semid, 1), "release parent");
  checkError(reserveSem(semid, 0), "reserve child");
  
  //detach memory
  checkError(shmdt(smap), "shmdt in child");
  
  checkError(releaseSem(semid, 1), "release parent");
  return 0;
}

int parentStuff(struct seg* smap, int semid, int shmid){
  int width, color;
  int cnt = 0;
  int printed = 0;

  // attach shared memory
  smap = shmat(shmid, NULL, 0);
  if (smap == (void *) -1) {
    checkError(-1, "shmat");
  }
  
  checkError(reserveSem(semid, 1), "reserve parent");

  // seed rand()
  srand(time(NULL));

  // generate width of output (30-40)
  width = (rand() % 11) + 30;
  
  // uncomment below if you wanna see the pairs and stuff
  // printf("Each line has %d characters.\n\n", width);

  // printf("Printing %d count-character pairs:\n", smap->randBlocks);
  // for (int i = 0; i < smap->randBlocks; i++){
  //   printf("(%d, %c) ", smap->blocks[i][0], smap->blocks[i][1]);
  // }
  // printf("\n\n");

  // print the top border
  printf("+");
  for (int i = 0; i < width; i++){
    printf("-");
  }
  printf("+\n|");

  // print characters
  for (int i = 0; i < smap->randBlocks; i++){
    color = rand() % 7 + 31;
    for (int j = 0; j < smap->blocks[i][0]; j++){
      // using ansi color codes
      printf("\e[0;%dm", color);
      printf("%c", smap->blocks[i][1]);
      printf("\e[0m");
      printed++;
      // check if width has been reached
      if (printed == width){
        printf("|\n|");
        printed = 0;
      }
    }
    cnt++;
    // check if all characters are printed
    if (cnt == smap->randBlocks){
      // add spaces if necessary, finish row
      for (int i = 0; i < width-printed; i++){
        printf(" ");
      }
      printf("|\n");
    }
  }

  // print the bottom border
  printf("+");
  for (int i = 0; i < width; i++){
    printf("-");
  }
  printf("+\n");

  checkError(releaseSem(semid, 0), "release child");
  checkError(reserveSem(semid, 1), "reserve parent");

  // detach memory
  checkError(shmdt(smap), "shmdt");
}


/* Problem 7 -- implement function main */

int main(int argc, char *argv[])
{
  struct seg *smap;
  union semun unSem;
  key_t semKey, shmKey;
  int semid, shmid, blocks;
  pid_t pid;

  // set keys for semaphore set and shared memory
  semKey = IPC_PRIVATE;
  shmKey = IPC_PRIVATE;

  // generate semaphore set
  semid = checkError(semget(semKey, 2, IPC_CREAT| OBJ_PERMS), "semget");

  // initialize child to available, parent to in use
  checkError(initSemAvailable(semid, 0), "initSemAvailable"); //child
  checkError(initSemInUse(semid, 1), "initSemInUse"); //parent

  // generate shared memory
  shmid = checkError(shmget(shmKey, sizeof(struct seg), IPC_CREAT | OBJ_PERMS), "shmget");

  // create child process, check for error
  pid = fork();
  if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

  // if in child...
  if (pid == 0){
    checkError(childStuff(smap, semid, shmid), "child function");
    return 0;
  }

  // else if in parent...
  else{
    checkError(parentStuff(smap, semid, shmid), "parent function");
  }

  // delete semaphore set and shared memory
  checkError(semctl(semid, 0, IPC_RMID), "semctl");
  checkError(shmctl(shmid, 0, IPC_RMID), "shmctl");

  exit(EXIT_SUCCESS);
}
