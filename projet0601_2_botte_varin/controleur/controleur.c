#include "structures.h"
#include "carte.h"

int stop=0;

pid_t minautores[5]={(pid_t)0}; /* stocker le pid de tous les minautores */
pid_t joueur=(pid_t)0; /* sotcker le pid du joueur */

void handler(int signum) {
    int i=0;
    stop=1;
    printf("\nEnvoie des signaux d'arrêt au joueur et minotaures s'il y a lieu.\n");
    if(joueur!=(pid_t)0) {
        printf("Envoie du signal au joueur\n");
        if(kill(joueur, SIGINT) == -1) {
            perror("Erreur lors de l'envoi du signal au joueur ");
            exit(EXIT_FAILURE);
        }
    }
    while(minautores[i]!=(pid_t)0) {
        printf("Envoie du signal aux minotaures...\n");
        if(kill(minautores[i], SIGINT) == -1) {
            fprintf(stderr,"Erreur lors de l'envoie du signal au minautore %d",i);
            perror(" ");
            exit(EXIT_FAILURE);
        }
        i++;
        printf("Tous les minotaures sont arrêtés.\n");
    }
}

/**
  * Fonction pour fermer tous les outils IPC ouvert.
  * @params cleFDM clé de la file de message
  * @params cleSHM clé du segment de mémoire partagé
  * @params cleTS clé du tableau de sémaphore
  */
void closeOuilsIPC(key_t cleFDM,key_t cleSHM, key_t cleTS) {

    int shmid,msqid,semid;

    /* Recuperation de la file */
    if((msqid = msgget(cleFDM, 0)) == -1) {
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
    if((shmid = shmget(cleSHM, 0, 0)) == -1) {
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
    if((semid = semget(cleTS, 0, 0)) == -1) {
      perror("Erreur lors de la recuperation du tableau de semaphores ");
      exit(EXIT_FAILURE);
    }
 
    /* Suppression du tableau de semaphores */
    if(semctl(semid, 0, IPC_RMID) == -1) {
      perror("Erreur lors de la suppresion du tableau de semaphores ");
      exit(EXIT_FAILURE);
    }
    printf("Tableau de semaphores supprime.\n");

    }
int main(int argc, char* argv[]) {
    int msqid,fd,i;

    segment_t * leSegment;


    struct sigaction action;

    reponse_t reponse;
    requete_t requete;

    int semid; /* semaphores id*/
    unsigned short val[3] = {0,1,5}; /* init des semaphores : autorisation connexion minotaure, nb joueur, nb minotaures */

    int numMinau=0;   

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
    printf("case 240 : %d\n",leSegment->cases[240].type-ZERO_ASCII);

    /* Creation du tableau de semaphore */
    if((semid = semget((key_t)atoi(argv[4]), 3, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
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

    /* Positionnement du gestionnaire pour SIGINT */
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGINT, &action, NULL) == -1) {
        perror("Erreur lors du positionnement ");
        exit(EXIT_FAILURE);
    }

    /* attente de connexion */
    while(!stop) {
        printf("En attente d'une requete...\n");
        if(msgrcv(msqid, &requete, sizeof(requete_t) - sizeof(long), 0, 0) == -1) {
            if(errno==EINTR) {break;}
            else {
                perror("Erreur lors de la reception d'une requete ");
                closeOuilsIPC((key_t)atoi(argv[2]),(key_t)atoi(argv[3]), (key_t)atoi(argv[3]));
                exit(EXIT_FAILURE);
            }
        }
        printf("Requete recue (%ld)\n", requete.type);
        switch(requete.type) {
            case TYPE_REQ_SMPTS:
                if(requete.typeActeur==TYPE_MINOTAURE) {
                    minautores[numMinau]=requete.pid;
                    numMinau++;
                }
                else if(requete.typeActeur==TYPE_JOUEUR) joueur=requete.pid;
                reponse.type = TYPE_REP_SMPTS;
                reponse.rep.r1.cleSMP = atoi(argv[3]);
                reponse.rep.r1.cleTS = atoi(argv[4]);
                if(msgsnd(msqid, &reponse, sizeof(reponse_t) - sizeof(long), 0) == -1) {
                    perror("Erreur lors de l'envoi de la reponse ");
                    exit(EXIT_FAILURE);
                }
                printf("Controleur : reponse envoyee.\n");
                break;
            case TYPE_REQ_DECO:
                if(requete.typeActeur==TYPE_JOUEUR) {
                    if(numMinau>0) {
                        printf("Envoie des demandes de déconnexion de tous les minautores...\n");
                        for(i=0;i<numMinau;i++)
                            {
                            if(kill(minautores[i], SIGINT) == -1) {
                                fprintf(stderr,"Erreur lors de l'envoi du signal au minautore %d",i);
                                perror(" ");
                                exit(EXIT_FAILURE);
                                }
                            }
                        printf("Tous les minautores déconnecté avec succès.\n");
                    }
                    if(joueur!=(pid_t)0) {
                        if(kill(joueur, SIGINT) == -1) {
                            perror("Erreur lors de l'envoi du signal au joueur ");
                            exit(EXIT_FAILURE);
                            }
                        }
                    printf("Controleur : reponse envoyee.\n");

                    /* on remet à 0 les valeurs */
                    memset(minautores,(pid_t)0,sizeof(pid_t)*5);
                    joueur = (pid_t)0;
                }
                else if(requete.typeActeur==TYPE_MINOTAURE) {
                    numMinau--;
                    minautores[numMinau]=(pid_t)0;
                }
            break;
        }
    }
    printf("Attente que tous les processus soit bien fini...\n");
    sleep(1);
    printf("Tous le monde est bien déconnecté, fermeture des outils IPC...\n");
    closeOuilsIPC((key_t)atoi(argv[2]),(key_t)atoi(argv[3]), (key_t)atoi(argv[3]));
    printf("Outils IPC fermés.\n");
    return EXIT_SUCCESS;
}