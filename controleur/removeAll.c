 
#include <stdlib.h>     /* Pour exit, EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h>      /* Pour printf, perror */
#include <sys/msg.h>    /* Pour msgget, msgctl */
#include <sys/shm.h>    /* Pour shmget, shmctl */
#include <sys/sem.h>    /* Pour semget, semctl */


int main(int argc, char * argv[])
    {
    int shmid,msqid,semid;

    if(argc!=4) {
        fprintf(stderr,"Usage : 3 argument\n");
        fprintf(stderr,"\tOu :\n");
        fprintf(stderr,"\t\tcle FDM : la clé de la file de message précédemment créer par le controleur\n");
        fprintf(stderr,"\t\tcle SHM : la clé du segment de mémoire partagé précédemment créer par le controleur\n");
        fprintf(stderr,"\t\tcle TS : la clé du tableau de semaphore récédemment créer par le controleur\n");
        exit(EXIT_FAILURE);
    }
    /* Recuperation de la file */
    if((msqid = msgget((key_t)atoi(argv[1]), 0)) == -1) {
        perror("Erreur lors de la recuperation de la file ");
        exit(EXIT_FAILURE);
    }
    
    /* Suppression de la file */
    if(msgctl(msqid, IPC_RMID, 0) == -1) {
        perror("Erreur lors de la suppression de la file ");
        exit(EXIT_FAILURE);
    }
    printf("File de messages supprime.\n");



    /* Recuperation du segment de memoire partagee */
    if((shmid = shmget(atoi(argv[2]), 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du segment de memoire partagee ");
        exit(EXIT_FAILURE);
    }
    
    /* Suppression du segment de memoire partagee */
    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("Erreur lors de la suppression du segment de memoire partagee ");
        exit(EXIT_FAILURE);
    }
    printf("Segment de memoire partagee supprime.\n");


      /* Recuperation du tableau de semaphores */
    if((semid = semget((key_t)atoi(argv[2]), 0, 0)) == -1) {
      perror("Erreur lors de la recuperation du tableau de semaphores ");
      exit(EXIT_FAILURE);
    }
 
    /* Suppression du tableau de semaphores */
    if(semctl(semid, 0, IPC_RMID) == -1) {
      perror("Erreur lors de la suppresion du tableau de semaphores ");
      exit(EXIT_FAILURE);
    }
    printf("Tableau de semaphores supprime.\n");
 
    return EXIT_SUCCESS;
    }
