#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>


using namespace std;

int main(int argc, char const *argv[])
{
    int dS = socket(PF_INET,SOCK_STREAM,0);
    if (dS ==-1){
        perror("error socket");
        exit(0);
    }
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));

    int id = bind(dS,(struct sockaddr*)&ad,sizeof(ad));
    if (id == -1){
        perror("bind error");
        exit(0);
    }

    int list = listen(dS,7);
    if (list == -1){
        perror("Listen error");
        exit(0);
    }


    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);


    while (1){
        int dSC = accept(dS,(struct sockaddr*)&aC,&lg);
        if (dSC == -1){
            perror("accept error");
            exit(0);
        }

        pid_t processus = fork();

        if (processus > 0) {
            
        }
        else if(processus == 0){ //PROCESSUS ENFANT

            int closedS = close(dS);
            if (closedS == -1){
                perror("closedS error");
                exit(0);
            }

            char msg[20];
            int rcv = recv(dSC,msg,sizeof(msg),0);
            if (rcv == -1){
                perror("recv error");
                exit(0);
            }

            cout<<"recu : "<<msg<<endl;
            int r = 10;
            int sd = send(dSC,&r,sizeof(int),0);
            if (sd == -1){
                perror("send error");
                exit(0);
            }

            int closeDSC = close(dSC);
            if (closeDSC == -1){
                perror("closeDSC error processus enfant");
                exit(0);
            }

            exit(0);
        }//FIN PROCESSUS enfant
        else{
            perror("erreur processus");
            exit(0);
        }

        /*int closeDSC = close(dSC);
        if (closeDSC == -1){
            perror("closeDSC error processus parent");
            exit(0);
        }*/
        
    }


    int closedS = close(dS);
    if (closedS == -1){
        perror("closedS error");
        exit(0);
    }

    return 0;
}
