#include "structures.h"
#include "carte.h"
#include <errno.h>
#include <signal.h>    /* Pour sigXXX */
#include <sys/types.h> /* Pour pid_t */

int stop=0;

void handler(int signum) {
    stop=1;
}

void sendDeco(requete_t requete,int msqid) {
    requete.type = TYPE_REQ_DECO;
    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requete ");
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char * argv[])
    {
    int msqid,shmid,semid;
    int ok=0;

    struct sigaction action;
    int position=0;
    requete_t requete;
    reponse_t reponse;
    struct sembuf op;

    unsigned char type=ZERO_ASCII;
    if(argc!=2) {
        fprintf(stderr,"Usage : 1 argument\n");
        fprintf(stderr,"\tOu :\n");
        fprintf(stderr,"\t\tcle FDM : la clé de la file de messgae précédemment créer par le controleur\n");
        exit(EXIT_FAILURE);
    }


    if((msqid = msgget((key_t)atoi(argv[1]), 0)) == -1) {
        perror("Erreur lors de la recuperation de la file ");
        exit(EXIT_FAILURE);
    }

    requete.type = TYPE_REQ_SMPTS;
    requete.typeActeur = TYPE_MINOTAURE;
    requete.pid = getpid();

    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requete ");
        exit(EXIT_FAILURE);
    }
    printf("Minautore : envoi requete pour cle SMP et TS.\n");
    printf("Minautore : en attente de la réponse...\n");
    if(msgrcv(msqid, &reponse, sizeof(reponse_t) - sizeof(long), TYPE_REP_SMPTS, 0) == -1) {
            perror("Erreur lors de la reception d'une requete ");
            sendDeco(requete,msqid);
            exit(EXIT_FAILURE);
        }


    /* Recuperation du segment de memoire partagee */
    if((shmid = shmget((key_t)reponse.rep.r1.cleSMP, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du segment de memoire partagee ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /* Recuperation du tableau de semaphores */
    if((semid = semget((key_t)reponse.rep.r1.cleTS, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du tableau de semaphores ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }
    printf("Minautore : reponse recu : clé : %d %d\n",reponse.rep.r1.cleSMP,reponse.rep.r1.cleTS);

     /* Recuperation de la valeur du semaphore */
    if((ok = semctl(semid, 0, GETVAL)) == -1) {
        perror("Erreur lors de la recuperation de la valeur du semaphore ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    if(!ok) {
        fprintf(stderr,"Le joueur n'est pas connécté.\n");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }
    /* savoir si le minautore à l'autorisation de se connecté */
    op.sem_num = 2;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        if(errno==EAGAIN)
            perror("Trop de minautore (>5) "); 
        else
            perror("Erreur lors de l'operation sur le semaphore ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /*changement d'un type d'une case en type minautore*/
    position = setCaseTypeMinotaure(shmid,semid,&type);
    
    /* Positionnement du gestionnaire pour SIGINT */
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGINT, &action, NULL) == -1) {
        perror("Erreur lors du positionnement ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    printf("Minautore: je suis là !\n");
    
    while(!stop) {
        bougerValMinotaure(shmid,position/30,position%30,&position,type);
        sleep(1);
    }
    
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if(semop(semid, &op, 1) == -1) {
        perror("Erreur lors de l'operation sur le semaphore ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    op.sem_num = 2;
    op.sem_op = 1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        perror("Erreur lors de l'operation sur le semaphore ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }
    
    sendDeco(requete,msqid);
    printf("Minautore : je me deco !\n");
    delCaseTypeMinotaure(shmid,position);

    return 0;
    }
