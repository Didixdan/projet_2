#include "structures.h"
#include "carte.h"
#include <errno.h>
#include <signal.h>    /* Pour sigXXX */
#include <sys/types.h> /* Pour pid_t */

int stop=0;

void handler(int signum) {
    stop=1;
}

int main(int argc, char * argv[])
    {
    int msqid,shmid,semid;

    struct sigaction action;
 
    requete_t requete;
    reponse_t reponse;
    struct sembuf op;

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

    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requete ");
        exit(EXIT_FAILURE);
    }
    printf("Minautore : envoi requete pour cle SMP et TS.\n");
    printf("Minautore : en attente de la réponse...\n");
    if(msgrcv(msqid, &reponse, sizeof(reponse_t) - sizeof(long), TYPE_REP_SMPTS, 0) == -1) {
            perror("Erreur lors de la reception d'une requete ");
            exit(EXIT_FAILURE);
        }


    /* Recuperation du segment de memoire partagee */
    if((shmid = shmget((key_t)reponse.rep.r1.cleSMP, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du segment de memoire partagee ");
        exit(EXIT_FAILURE);
    }

    /* Recuperation du tableau de semaphores */
    if((semid = semget((key_t)reponse.rep.r1.cleTS, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du tableau de semaphores ");
        exit(EXIT_FAILURE);
    }
    printf("Minautore : reponse recu : clé : %d %d\n",reponse.rep.r1.cleSMP,reponse.rep.r1.cleTS);
    
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        if(errno==EAGAIN)
            perror("Nombre maximal de minautore atteint "); 
        else
            perror("Erreur lors de l'operation sur le semaphore ");
        exit(EXIT_FAILURE);
    }
	
    /*changement d'un type d'une case en type minautore*/
    setCaseTypeMinotaure(shmid);
    
    /* Positionnement du gestionnaire pour SIGINT */
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGINT, &action, NULL) == -1) {
        perror("Erreur lors du positionnement ");
        exit(EXIT_FAILURE);
    }

    printf("Minautore : je suis là !\n");

    while(!stop)pause();

    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if(semop(semid, &op, 1) == -1) {
        perror("Erreur lors de l'operation sur le semaphore ");
        exit(EXIT_FAILURE);
    }
    printf("Minautore : je me deco !\n");
    return 0;
    }
