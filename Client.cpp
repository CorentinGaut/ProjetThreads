#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>


using namespace std;

pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

struct Memoire {
    int nbPlace;
};

struct Socket{
    int dS;
    struct sockaddr_in aS;
    socklen_t lgA;
    const char* message;
};

void *Recoit (void *par){
    struct Socket * resg  = (struct Socket*)(par);
    int c;
    cout<< "Dans le thread"<<endl;
    int rcv = recv(resg->dS,&c,sizeof(int),0);
    if (rcv == -1){
        perror("recu error");
        exit(0);
    }
    cout<<"reponse : "<<c<<endl;

    pthread_exit(NULL);

}

void *Envoyer (void *par){
    struct Socket * resg  = (struct Socket*)(par);
    
    bool quit = true;

while(quit)
{
    cout<<"Wesh gros bien ou bien?"<<endl;
    cout<< "Qu'est tu veux faire?"<<endl<<endl;
    cout<< "1 : Ecrire un message dans une case?"<<endl<<"2 : ";
    int snd = send(resg->dS,resg->message,sizeof(resg->message),0);
}

    if (snd == -1){
        perror("send error");
        exit(0);
    }

    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{

    struct Socket buffer;

    pthread_t idThRcv;
    pthread_t idThSend;

    //initialiser la mémoire 
    key_t sesame = ftok("Clef",1);

    int shm = shmget(sesame,sizeof(Memoire),IPC_CREAT|0666);
    if (shm == -1){
        perror("ShmeGet error");
        exit(1);
    }

    //met la memoire dans un buffer
    struct Memoire * bufferMem = (struct Memoire*)shmat(shm, NULL, 0);

    //creation de la socket
    buffer.dS = socket(PF_INET,SOCK_STREAM,0);
    if (buffer.dS  ==-1){
        perror("error socket");
        exit(0);
    }

    buffer.aS.sin_family = AF_INET;
    buffer.aS.sin_addr.s_addr = INADDR_ANY;
    buffer.aS.sin_port = htons(atoi(argv[1]));
    buffer.lgA = sizeof(struct sockaddr_in);

    int conct = connect(buffer.dS,(struct sockaddr*)&buffer.aS,buffer.lgA);
    if (conct == -1){
        perror("connect error");
        exit(0);
    }

    buffer.message = argv[2];
    cout<< "lancement du threads"<<endl;
    //creer le threads du receive
    


    /*string m = "Bonjour";
    char * c = strdup(m.c_str());
    int snd = send(buffer.dS,c,sizeof(m),0);
    if (snd == -1){
        perror("send error");
        exit(0);
    }*/

    pthread_create(&idThRcv,NULL,Recoit,&buffer); //TODO: mettre en parametre tout le bordel des sockets 

    pthread_create(&idThSend,NULL,Envoyer,&buffer);

    //fin du threads
    pthread_join(idThRcv,NULL);
    pthread_join(idThSend,NULL);


    int r;
    /*int rcv = recv(buffer.dS,&r,sizeof(int),0);
    if (rcv == -1){
        perror("recv error");
        exit(0);
    }
    cout<<"reponse : "<<r<<endl;*/

    int closedS = close(buffer.dS);
    if (closedS == -1){
        perror("closedS error");
        exit(0);
    }

}
