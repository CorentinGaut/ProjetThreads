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
#include "structures.h"


struct a_recevoir {
    int socket_server;
    char tab [TAILLE_TAB][NB_CARAC];
};

void *update(void * param){

  struct a_recevoir *recoie = (struct a_recevoir*) param;
  struct message msg;

  //int res = recv(recoie->socket_server,recoie->tab,sizeof(char)*TAILLE_TAB*NB_CARAC,0);
  /*if(res == -1){
    perror("recv from server ");
    exit(-1);
  } else if(res == 0){
    printf("Serveur déconnecté\n");
    exit(0);
  }
  displayTab(recoie->tab);*/

  while(1){
      int res = recv(recoie->socket_server,recoie->tab,sizeof(char)*TAILLE_TAB*NB_CARAC,0);
      if(res == -1){
        perror("recv from server ");
        exit(-1);
      } else if(res == 0){
        printf("Serveur déconnecté\n");
        exit(0);
      }
      printf("message recu\n");
      strcpy(recoie->tab[msg.i],msg.msg);
      strcpy(msg.msg, "");
      printf("message apres vidage = %s\n", msg.msg);
      displayTab(recoie->tab);
  }
}

int main(int argc, char const *argv[])
{
    //Vérifie le nombre d'argument passé en commande
    if(argc != 3){
        printf("Utilisation ./client addresseIP numPort  %d\n",argc);
        exit(0);
    }

    //On créé la socket
    int sockServeur = socket(PF_INET,SOCK_STREAM,0);
    if(sockServeur == -1){
        perror("socket ");
        exit(-1);
    }

    socklen_t lAddr = sizeof(struct sockaddr_in);

    //On créé la structure pour avoir l'adresse du serveur
    struct sockaddr_in adServeur;
    adServeur.sin_family = AF_INET;
    inet_pton(AF_INET,argv[1],&(adServeur.sin_addr));
    adServeur.sin_port = htons((short) atoi(argv[2]));

    //On demande une connection au serveur
    int res = connect(sockServeur,(struct sockaddr *) &adServeur, lAddr);
    if(res == -1){
        perror("connect ");
        exit(-1);
    }
    printf("Connection établie\n");

    struct message msg;
    struct a_recevoir arcv;

    arcv.socket_server = sockServeur;

    pthread_t id;
    pthread_create(&id,NULL,update,&arcv);
    //On peut échanger avec le serveur 
    while(1){
      printf("Quel index du tableau : ");
      scanf("%d",&msg.i);
      printf("entrez votre message : ");
      scanf("%s",msg.msg);
      printf("message a envoyé = %s\n", msg.msg);
      res = send(sockServeur,&msg,sizeof(char)*NB_CARAC+ sizeof(int)*2,0);
      if(res == -1){
        perror("send to server ");
        exit(-1);
      } else if(res == 0){
        printf("Serveur déconnecté\n");
        exit(0);
      }
      //strcpy(msg.msg, "");
    }

    //On ferme la socket créé
    res = close(sockServeur);
    if(res == -1){
        perror("close ");
        exit(-1);
    }

    return 0;
}
