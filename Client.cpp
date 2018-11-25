#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <string.h>

using namespace std;

int main(int argc, char const *argv[])
{

    int dS = socket(PF_INET,SOCK_STREAM,0);
    if (dS ==-1){
        perror("error socket");
        exit(0);
    }
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    aS.sin_addr.s_addr = INADDR_ANY;

    aS.sin_port = htons(atoi(argv[1]));
    socklen_t lgA = sizeof(struct sockaddr_in);

    int conct = connect(dS,(struct sockaddr*)&aS,lgA);
    if (conct == -1){
        perror("connect error");
        exit(0);
    }

    string m = "Bonjour";
    char * c = strdup(m.c_str());
    int snd = send(dS,c,sizeof(m),0);
    if (snd == -1){
        perror("send error");
        exit(0);
    }

    int r;
    int rcv = recv(dS,&r,sizeof(int),0);
    if (rcv == -1){
        perror("recv error");
        exit(0);
    }
    cout<<"reponse : "<<r<<endl;

    int closedS = close(dS);
    if (closedS == -1){
        perror("closedS error");
        exit(0);
    }

}
