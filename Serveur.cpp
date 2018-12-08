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
#include "structures.h"

void *miseAJour(void * param){
  struct client *structClient = (struct client *) param;
  int valSem;

  opP(structClient->idSem,NB_SEMA-1);
  int res = send(structClient->sockClient,structClient->tabMem->tab,sizeof(char)* TAILLE_TAB * NB_CARAC,0);
  if(res == -1){
    perror("recv from client ");
    exit(-1);
  } else if(res == 0){
    printf("Client déconnecté\n");
    deconnectionClient(structClient->idSem, structClient->tabMem);
    exit(0);
  }
  //displayTab(structClient->tabMem.tab);
  opV(structClient->idSem,NB_SEMA-1);

  while (1) {
    for(int i = 0; i < NB_SEMA-1; i++){
      if((valSem = semctl(structClient->idSem,i,GETVAL)) == -1){
        perror("semctl ");
        exit(-1);
      }
      opP(structClient->idSem,NB_SEMA-1);
      int nbClients = structClient->tabMem->nbClients;
      opV(structClient->idSem,NB_SEMA-1);
      //printf("valSem = %d et nbClient+1 = %d pour le semaphore %d\n",valSem,nbClients+1,i);
      if(valSem < nbClients + 1){
        opP(structClient->idSem,i);
        struct message msg;
        msg.i = i / TAILLE_TAB ;
        //printf("Je vais envoyer le message = %s pour la case %d %d\n",structClient->tabMem->tab[msg.i][msg.j],msg.i,msg.j);
        strcpy(msg.msg,structClient->tabMem->tab[msg.i]);
        res = send(structClient->sockClient,&msg,sizeof(int)*2 + sizeof(char)*NB_CARAC /*strlen(msg.msg)*/,0);
        if(res == -1){
          perror("recv from client ");
          exit(-1);
        } else if(res == 0){
          printf("Client déconnecté\n");
          deconnectionClient(structClient->idSem, structClient->tabMem);
          exit(0);
        }
        //opZ(structClient->idSem,i);
        printf("message envoyé pour la case %d et message : %s\n",msg.i,msg.msg);
        opV(structClient->idSem,i);
      }
    }
  }
}

int main(int argc, char const *argv[])
{
    //Vérifie le nombre d'argument passé en commande
    if(argc != 2){
        printf("Utilisation ./servtabMemoire");
        exit(0);
    }

    //On créé la structure de donntabMemoire
    struct tabMemoire tabMem;
    tabMem.nbClients = 0;

    for(int i = 0; i < TAILLE_TAB; i++){
      for(int j = 0; j < TAILLE_TAB; j++){
        strcpy(tabMem.tab[i],"");
      }
    }

    //On créé une clé pour les IPCtabMemoire
    key_t cle = ftok("readme.txt",32);
    if(cle == -1){
        perror("ftok ");
        exit(-1);
    }

    //Création du segment de mémoire partagé
    int id_seg = shmget(cle,sizeof(struct tabMemoire),IPC_CREAT|0666);
    if(id_seg == -1){
        perror("shmget ");
        exit(-1);
    }

    //On récupère le pointeur vers le segment de mémoire
    struct tabMemoire *tabParta = (struct tabMemoire *)shmat(id_seg,NULL,0);
    if(tabParta == (void *) -1){
        perror("shmat ");
        exit(-1);
    }
    *tabParta = tabMem;


    //Création d'un tableau de sémaphore
    int id_sem = semget(cle,NB_SEMA,IPC_CREAT|0666);
    if(id_sem == -1){
        perror("semget ");
        exit(-1);
    }

    //Initialisation des sémaphores
    for(int i = 0; i < NB_SEMA; i++){
        if(semctl(id_sem,i,SETVAL,1) == -1){
            perror("semctl ");
            exit(-1);
        }
    }

    //On créé la socket
    int sockServeur = socket(PF_INET,SOCK_STREAM,0);
    if(sockServeur == -1){
        perror("socket ");
        exit(-1);
    }

    socklen_t lAddr = sizeof(struct sockaddr_in);

    //On créé la structure pour avoir l'adresse du client
    struct sockaddr_in adClient;
    adClient.sin_family = AF_INET;
    adClient.sin_addr.s_addr = INADDR_ANY;
    adClient.sin_port = htons((short) atoi(argv[1]));

    //On bind la socket
    int res = bind(sockServeur, (struct sockaddr *) &adClient, lAddr);
    if(res == -1){
        perror("bind ");
        exit(-1);
    }

    //On écoute la socket
    res = listen(sockServeur, 5);
    if(res == -1){
        perror("listen ");
        exit(-1);
    }

    while(1){
        //On accepte une demande de connection d'un client
        int sockClient = accept(sockServeur, (struct sockaddr *) &adClient, &lAddr);
        if(sockClient == -1){
            perror("accept ");
        exit(-1);
        }
        printf("Connection accepté \n");

        pid_t pid = fork();
        if(pid == 0){ //On est dans le fils
            //On ferme la socket du serveur
            pthread_t idT;
            res = close(sockServeur);
            if(res == -1){
                perror("close ");
                exit(-1);
            }
            printf("Transféré au fils\n");

            opP(id_sem,NB_SEMA-1);

            tabParta->nbClients += 1;

            for(int i = 0; i < NB_SEMA-1; i++){
              if(semctl(id_sem,i,SETVAL,semctl(id_sem,i,GETVAL)+1) == -1){
                perror("semctl ");
                exit(-1);
              }
            }

            opV(id_sem,NB_SEMA-1);

            struct client adClient;
            adClient.sockClient = sockClient;
            adClient.lAddr = lAddr;
            adClient.tabMem = tabParta;
            adClient.idSem = id_sem;

            pthread_create(&idT,NULL,miseAJour,&adClient);
            //Faire les échanges entre client/serveur
            struct message msg;

            while(1){
              //printf("msg = %s\n", msg.msg);
              res = recv(sockClient,&msg,sizeof(char) * NB_CARAC + sizeof(int)* 2, 0);
              if(res == -1){
                perror("recv from client ");
                exit(-1);
              } else if(res == 0){
                printf("Client déconnecté\n");
                deconnectionClient(id_sem, tabParta);
                exit(0);
              }
              opP(id_sem,msg.i * TAILLE_TAB);
              strcpy(tabParta->tab[msg.i],msg.msg);
              strcpy(msg.msg,"");
              printf("message = %s\n", adClient.tabMem->tab[msg.i]);
              if(semctl(id_sem,msg.i * TAILLE_TAB,SETVAL,tabParta->nbClients) == -1){
                perror("semctl ");
                exit(-1);
              }
              printf("modification de la case %d avec les message : %s\n",msg.i,tabParta->tab[msg.i]);
              //displayTab(tabParta->tab);
              opZ(id_sem,msg.i * TAILLE_TAB);
              opV(id_sem,msg.i * TAILLE_TAB);
            }

            //phtread_join(&idT,NULL);
            exit(0);
        } else { //On est dans le père
            //On ferme la socket du client
            res = close(sockClient);
            if(res == -1){
                perror("close ");
                exit(-1);
            }
        }
    }

    //On ferme la socket du serveur
    res = close(sockServeur);
    if(res == -1){
        perror("close ");
        exit(-1);
    }

    return 0;
}
