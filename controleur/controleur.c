#include "structures.h"
#include "carte.h"


int stop=0;

int main(int argc, char* argv[]) {
    int msqid,fd;

    segment_t * leSegment;

    reponse_t reponse;
    requete_t requete;

    int semid; /* semaphores id*/
    unsigned short val[2] = {5, 1}; /* init des semaphores : minautore,joueur */

    /* Test arguments */
    if(argc!=5) {
        fprintf(stderr,"Usage : 4 arguments\n");
        fprintf(stderr,"\tOu :\n");
        fprintf(stderr,"\t\tnomCarte : le nom de la carte à lancer\n");
        fprintf(stderr,"\t\tcle FDM  : la clé de la file de message\n");
        fprintf(stderr,"\t\tcle SMP : la clé du segment de mémoire partagé\n");
        fprintf(stderr,"\t\tcle TS : la clé du tableau de sémaphore\n");
        exit(EXIT_FAILURE);
    }

    /* Création file message */
    if((msqid = msgget((key_t)atoi(argv[2]), S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST)
            fprintf(stderr, "Erreur : file (cle=%d) existante\n", atoi(argv[2]));
        else
            perror("Erreur lors de la creation de la file ");
        exit(EXIT_FAILURE);
    }
    printf("File de message cree.\n");

    /* Création segment de mémoire partagée contenant la carte */
    fd = openCarte(argv[1]);
    leSegment = creer_segment((key_t)atoi(argv[3]),getTerrainCarte(fd));
    printf("Segment cree. test : %d\n",*(leSegment->largeur));
    printf("case 241 : %d\n",leSegment->cases[241].type-ZERO_ASCII);

    /* Attachement du segment de memoire partagee 
    if((adresse = shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de memoire partagee");
        exit(EXIT_FAILURE);
    }

     Detachement du segment de memoire partagee 
    if(shmdt(adresse) == -1) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }*/


    /* Creation du tableau de semaphore */
    if((semid = semget((key_t)atoi(argv[4]), 2, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST)
            fprintf(stderr, "Tableau de semaphores (cle=%d) existant\n", atoi(argv[4]));
        else
            perror("Erreur lors de la creation du tableau de semaphores ");
        exit(EXIT_FAILURE);
    }

    /* Initialisation des semaphores */
    if(semctl(semid, 0, SETALL, val) == -1) {
        perror("Erreur lors de l'initialisation des semaphores ");
        exit(EXIT_FAILURE);
    }
    printf("Tableau de semaphore cree.\n");
    /***/

    /* attente de connexion */
    while(!stop) {
        printf("En attente d'une requete...\n");
        if(msgrcv(msqid, &requete, sizeof(requete_t) - sizeof(long), TYPE_REQ_SMPTS, 0) == -1) {
            perror("Erreur lors de la reception d'une requete ");
            exit(EXIT_FAILURE);
        }
        printf("Requete recue (%ld)\n", requete.type);
        switch(requete.type) {
            case TYPE_REQ_SMPTS:
                reponse.type = TYPE_REP_SMPTS;
                reponse.rep.r1.cleSMP = atoi(argv[3]);
                reponse.rep.r1.cleTS = atoi(argv[4]);
                if(msgsnd(msqid, &reponse, sizeof(reponse_t) - sizeof(long), 0) == -1) {
                    perror("Erreur lors de l'envoi de la reponse ");
                    exit(EXIT_FAILURE);
                }
                printf("Controleur : reponse envoyee.\n");
                break;
            default:
                printf("ah");
                break;
        }
        
    }

    return EXIT_SUCCESS;
}