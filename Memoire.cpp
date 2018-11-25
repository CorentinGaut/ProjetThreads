#include <iostream>
#include <sys/shm.h>


struct Memoire {
    int nbPlace;
};

int main(int argc, char const *argv[])
{

    //initialiser la mémoire 
    key_t sesame = ftok("Clef",1);

    int shm = shmget(sesame,sizeof(Memoire),IPC_CREAT|0666);
    if (shm == -1){
        perror("ShmeGet error");
        exit(1);
    }


    return 0;
}
