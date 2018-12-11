
#include "structures.h"
#include <iostream>

using namespace std;

void *miseAJour(void * param){
  struct client *structClient = (struct client *) param;
  int valSem;

  opP(structClient->idSem,NB_SEMA-1); //premier envoie du tableau lors de la connection du client
  int res = send(structClient->sockClient,structClient->tabMem->tab,sizeof(char)*TAILLE_TAB*NB_CARAC,0);
  if(res == -1){
    perror("recv client maj");
    exit(-1);
  } else if(res == 0){
    cout<<"deconnection client "<<endl;
    deconnectionClient(structClient->idSem, structClient->tabMem);
    exit(0);
  }
  opV(structClient->idSem,NB_SEMA-1);

  while (1) { // boucle de misa a jour
    for(int i = 0; i < NB_SEMA-1; i++){
      if((valSem = semctl(structClient->idSem,i,GETVAL)) == -1){ // verification du numero de semaphore
        perror("semctl maj ");
        exit(-1);
      }
      //cout<<"opp maj nbclient"<<endl;
      opP(structClient->idSem,NB_SEMA-1); // recupération du nombre de clients
      int nbClients = structClient->tabMem->nbClients;
      opV(structClient->idSem,NB_SEMA-1);
      if(valSem <= nbClients)// test pour le numero de semaphore pour savoir si il y a eu une mise a jour
      { 
            cout<<"opp maj case tab"<<endl;

        opP(structClient->idSem,i);  //si oui, on bloque le semaphore
        struct message msg;
        msg.i = i / TAILLE_TAB ;
        strcpy(msg.msg,structClient->tabMem->tab[msg.i]); 
        res = send(structClient->sockClient,&msg,sizeof(int)*2 + sizeof(char)*NB_CARAC ,0);
        if(res == -1){
          perror("recv from client ");
          exit(-1);
        } else if(res == 0){
          cout<<"deconnection client "<<endl;
          deconnectionClient(structClient->idSem, structClient->tabMem);
          exit(0);
        }
        //mettre un rendez-vous opz;
        cout<<"message envoyé pour la case "<<msg.i<< " et message : "<<msg.msg<<endl;
        opV(structClient->idSem,i);
      }
    }
  }
}

int main(int argc, char const *argv[])
{
    //Vérifie le nombre d'argument passé en commande
    if(argc != 2){
        cout<<"Utilisation ./servtabMemoire"<<endl;
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
    key_t cle = ftok("Clef",32);
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
            perror("semctl2 ");
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
        cout<<"Connection accepté "<<endl;

        pid_t pid = fork();
        if(pid == 0){ //On est dans le fils
            //On ferme la socket du serveur
            pthread_t idT;
            res = close(sockServeur);
            if(res == -1){
                perror("close ");
                exit(-1);
            }
            cout<<"Transféré au fils "<<endl;
            cout<<"opp main connectionclient"<<endl;

            opP(id_sem,NB_SEMA-1);

            tabParta->nbClients += 1;

            for(int i = 0; i < NB_SEMA-1; i++){
              if(semctl(id_sem,i,SETVAL,semctl(id_sem,i,GETVAL)+1) == -1){
                perror("semctl3 ");
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
                    cout<<"Client déconnecté"<<endl;
                    deconnectionClient(id_sem, tabParta);
                    exit(0);
                }
                cout<<"opv recption msg client"<<endl;
                opP(id_sem,msg.i );
                strcpy(tabParta->tab[msg.i],msg.msg);
                strcpy(msg.msg,"");
                cout<<"message = "<<adClient.tabMem->tab[msg.i]<<endl;
                if(semctl(id_sem,msg.i ,SETVAL,tabParta->nbClients) == -1){
                    //printf("semctl4 : %d, %d, %d", id_sem, msg.i * TAILLE_TAB, tabParta->nbClients);
                    perror("semctl4");
                    exit(-1);
                }
                cout<<"modification de la case "<< msg.i<<" avec les message : "<<tabParta->tab[msg.i]<<endl;
                displayTab(tabParta->tab);
                //opZ(id_sem,msg.i * TAILLE_TAB);
                opV(id_sem,msg.i );
            }

            pthread_join(idT,NULL);
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
