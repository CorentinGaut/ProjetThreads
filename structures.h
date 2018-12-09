#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <limits.h>
#include <string.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <errno.h>

#define NB_CARAC 20
#define TAILLE_TAB 5
#define NB_SEMA TAILLE_TAB + 1

struct sembuf sempar;

struct tabMemoire {
    char tab[TAILLE_TAB][NB_CARAC];
    int nbClients;
};

struct message {
    int i;
    char msg[NB_CARAC];
};

struct client {
    int sockClient;
    struct sockaddr_in addrClient;
    struct tabMemoire *tabMem;
    socklen_t lAddr;
    int idSem;
};

void opP(int semid,int sem_num) {

  sempar.sem_num = sem_num ;
  sempar.sem_op = -1 ;
  sempar.sem_flg = 0 ; //0 ou SEM_UNDO

  if (semop(semid,&sempar,1) == -1)
    perror("Erreur lors du p") ;
}

void opV(int semid, int sem_num) {

  sempar.sem_num = sem_num ;
  sempar.sem_op = +1 ;
  sempar.sem_flg = 0 ;  // 0 ou SEM_UNDO

  if (semop(semid,&sempar,1) ==-1)
    perror("Erreur lors du v") ;
}

void opZ(int semid, int sem_num) {

  sempar.sem_num = sem_num ;
  sempar.sem_op = 0 ;
  sempar.sem_flg = 0 ;  // 0 ou SEM_UNDO
  if (semop(semid,&sempar,1) == -1){
    if(errno != EAGAIN)
      perror("Erreur lors du v") ;
  } else {
    //opV(semid, sem_num);
  }
}

void displayTab(char tab[TAILLE_TAB][NB_CARAC]){
    printf("[");
    for(int i=0;i<TAILLE_TAB;i++){
      printf("%s\t",tab[i]);
    }
    printf("]\n");
}

void deconnectionClient(int idSem, struct tabMemoire *tab){
  opP(idSem,NB_SEMA-1);
  tab->nbClients -= 1;
  for(int i = 0; i < NB_SEMA-1; i++){
    if(semctl(idSem,i,SETVAL,semctl(idSem,i,GETVAL)-1) == -1){
      perror("semctl ");
      exit(-1);
    }
  }
  opV(idSem,NB_SEMA-1);
}
