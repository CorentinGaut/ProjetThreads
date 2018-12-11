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
#include <iostream>
#include "structures.h"

using namespace std;


struct a_recevoir {
    int socket_server;
    char tab [TAILLE_TAB][NB_CARAC];
};

void *update(void * param){

  struct a_recevoir *recoie = (struct a_recevoir*) param;
  struct message msg;

  int res = recv(recoie->socket_server,recoie->tab,sizeof(char)*TAILLE_TAB*NB_CARAC,0);
  if(res == -1){
    perror("recv from server ");
    exit(-1);
  } else if(res == 0){
    printf("Serveur déconnecté\n");
    exit(0);
  }
  displayTab(recoie->tab);

  //while(1){
      int res1 = recv(recoie->socket_server,recoie->tab,sizeof(char)*TAILLE_TAB*NB_CARAC,0);
      if(res1 == -1){
        perror("recv from server ");
        exit(-1);
      } else if(res == 0){
        cout<<"Serveur déconnecté\n";
        exit(0);
      }
      cout<<"message recu\n";
      strcpy(recoie->tab[msg.i],msg.msg);
      strcpy(msg.msg, "");
      cout<<"tableau = "<<endl<< msg.msg;
      displayTab(recoie->tab);
  //}
  pthread_exit(0);
}

int main(int argc, char const *argv[])
{
    //Vérifie le nombre d'argument passé en commande
    if(argc != 3){
        cout<<"passez en paramètre l'addresseIP et le numero de port"<<endl;
        exit(-1);
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
    cout<<"Connection établie"<<endl;

    struct message msg;
    struct a_recevoir arcv;

    arcv.socket_server = sockServeur;

    pthread_t id;
    bool quit=true;
    pthread_create(&id,NULL,update,&arcv);
    //On peut échanger avec le serveur 
    while(quit){
      int choix=0;
      cout<<"Que veux tu faire? \n\t 1 : Ecrire dans le tableau\n\t 2 : Quitter \n\t 3 : Afficher le tableau"<<endl;
      cin>>choix;
      switch(choix)
      {
        case 1 : 
        {
          cout<<"Quel index du tableau? (max "<<TAILLE_TAB<<")"<<endl;
          cin>>msg.i;
          if(msg.i>=TAILLE_TAB)
          {
            cout<<"index innaccessible"<<endl;
            break;
          }
          //string input;
          cout<<"entrez votre message (20 caracteres max): "<<endl;
          scanf("%s",msg.msg);
          // cin>>input;
          // if(sizeof(input)>NB_CARAC*sizeof(char))
          // {
          //   cout<<"nombre de caracteres invalide"<<endl;
          //   break;
          // }
          // for(int i=0;i<20;i++)
          // msg.msg[i]=input[i];
          cout<<"message a envoyé = "<< msg.msg<<endl;
          res = send(sockServeur,&msg,sizeof(char)*NB_CARAC+ sizeof(int)*2,0);
                if(res == -1){
          perror("send to server ");
          exit(-1);
          } else if(res == 0){
            printf("Serveur déconnecté\n");
            exit(0);
          }
          break;
        }
        case 2 :
        { //mutex a rajouter
          pthread_join(id,NULL);
          //On ferme la socket créé
          res = close(sockServeur);
          if(res == -1){
              perror("close ");
              exit(-1);
          }
          return 0;
        }
        case 3 :
        {
          break;
        }
      }
    }


}
