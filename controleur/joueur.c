#include "structures.h"
#include "carte.h"
#include <errno.h>
#include <signal.h>    /* Pour sigXXX */
#include <sys/types.h> /* Pour pid_t */

#define LARGEUR 75      /* Largeur de la fenêtre */
#define HAUTEUR 27      /* Hauteur de la fenêtre */
#define POSX    20      /* Position horizontale de la fenêtre */
#define POSY    5       /* Position verticale de la fenêtre */

int stop = 0;

void handler(int signum){
        stop = 1;
    }

int main(int argc, char * argv[])
    {
    
    WINDOW *jeuCarte,*jeuFen, *infosFen,*infosText, *buttonFen;
    int infosFenY = 10; /* taille en Y de la fenetre */  

    int msqid,shmid,semid;
    requete_t requete;
    reponse_t reponse;
    int ch,posX,okMove=0;
    int vPosX=0,vPosY=8;
    int vPosXTemp,vPosYTemp;
    char * msgDirect;
    char * msgInfo;
    struct sembuf op;
    struct sigaction action;
    segment_t *leSegment;

    if(argc!=2) {
        fprintf(stderr,"Usage : 1 argument\n");
        fprintf(stderr,"\tOu :\n");
        fprintf(stderr,"\t\tcle FDM : la clé de la file de messgae précédemment créer par le controleur\n");
        exit(EXIT_FAILURE);
    }

    /* Initialisation de ncurses */
    ncurses_initialiser();
    ncurses_souris();
    ncurses_couleurs();

    /* Vérification de la taille du terminale */
    if((COLS < POSX + LARGEUR) || (LINES < POSY + HAUTEUR)) {
      ncurses_stopper();
      fprintf(stderr, "Les dimensions du terminal sont insufisantes : l=%d,h=%d au lieu de l=%d,h=%d\n", COLS, LINES, POSX + LARGEUR, POSY + HAUTEUR);
      exit(EXIT_FAILURE);
    }
    /*printf("*****1*****\n\n\n\n\n\n");*/
    if((msqid = msgget((key_t)atoi(argv[1]), 0)) == -1) {
        perror("Erreur lors de la recuperation de la file ");
        exit(EXIT_FAILURE);
    }

    requete.type = TYPE_REQ_SMPTS;
    requete.typeActeur = TYPE_JOUEUR;
    requete.pid = getpid();
    /*printf("*****2*****\n\n\n\n\n\n");*/

    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requete ");
        exit(EXIT_FAILURE);
    }
    
    /*printf("*****3*****\n\n\n\n\n\n");*/
    if(msgrcv(msqid, &reponse, sizeof(reponse_t) - sizeof(long), TYPE_REP_SMPTS, 0) == -1) {
            perror("Erreur lors de la reception d'une requete ");
            exit(EXIT_FAILURE);
        }

    /*printf("******4*****\n");*/
    /* Recuperation du segment de memoire partagee */
    if((shmid = shmget((key_t)reponse.rep.r1.cleSMP, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du segment de memoire partagee ");
        exit(EXIT_FAILURE);
    }
    /*printf("*****5******\n");
    printf("******6 %d*****\n",reponse.rep.r1.cleTS);*/
    /* Recuperation du tableau de semaphores */
    if((semid = semget((key_t)reponse.rep.r1.cleTS, 0, 0)) == -1) {
        perror("Erreur lors de la recuperation du tableau de semaphores ");
        exit(EXIT_FAILURE);
    }
    /*printf("*****7******\n");*/

    /* donner l'information de l'autorisation de connexion pour les minautores*/
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        if(errno==EAGAIN)
            perror("Impossible de donner l'autorisation "); 
        else
            perror("Erreur lors de l'operation sur le semaphore ");
        exit(EXIT_FAILURE);
    }

    /*réalisation de P(S1,1)*/
    op.sem_num = 1;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1){
        if(errno == EAGAIN)
            perror("Nombre maximal de joueur atteint ");
        else
            perror("Erreur lors de l'opération sur le semaphore.");
        exit(EXIT_FAILURE);
    }

    /*mise en place du gestionanire pour SIGINT*/
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGINT, &action, NULL) == -1){
        perror("Erreur de positionnement ");
        exit(EXIT_FAILURE);
    }
    
    /*Calcul de la position centrale*/
    posX = COLS / 2 - 1;
    move(0,0);
    
    infosFen = newwin(infosFenY, posX, 0, 0);
    mvwaddch(infosFen, 0, 0, ' ');

    infosText = derwin(infosFen,8, posX-2,1,1);
    scrollok(infosText, TRUE);
    


    buttonFen = newwin(8,18,11,LARGEURC+3);
    wmove(buttonFen,0,1);
    wprintw(buttonFen,"Vies : %c",getNbVie(shmid));
    wbkgd(buttonFen,COLOR_PAIR(15));

    /* affichage de la légende */
    wmove(buttonFen,1,1);
    wprintw(buttonFen,"Deplacement : %d",getNbVal(shmid, 4));

    wmove(buttonFen,2,1);
    wprintw(buttonFen,"Mur : %d/%d",getNbVal(shmid, 3), getNbVal(shmid, 1)+getNbVal(shmid, 3));


    wattron(buttonFen,COLOR_PAIR(12));
    mvwaddch(buttonFen,3,1,' ');
    wattroff(buttonFen,COLOR_PAIR(12));
    wmove(buttonFen,3,3);
    wprintw(buttonFen,"Mur visible");


    wattron(buttonFen,COLOR_PAIR(9));
    mvwaddch(buttonFen,4,1,' ');
    wattroff(buttonFen,COLOR_PAIR(9));
    wmove(buttonFen,4,3);
    wprintw(buttonFen,"Guerrier");

    wattron(buttonFen,COLOR_PAIR(13));
    mvwaddch(buttonFen,5,1,' ');
    wattroff(buttonFen,COLOR_PAIR(13));
    wmove(buttonFen,5,3);
    wprintw(buttonFen,"Mur decouvert");

    wattron(buttonFen,COLOR_PAIR(1));
    mvwaddch(buttonFen,6,1,'S');
    wattroff(buttonFen,COLOR_PAIR(1));
    wmove(buttonFen,6,3);
    wprintw(buttonFen,"Sortie");
    /***********************************/

    wrefresh(buttonFen);
    
    jeuFen = newwin(HAUTEURC+2,LARGEURC+2,11,0);
    box(jeuFen, 0, 0);
    wbkgd(jeuFen,COLOR_PAIR(15));
    wrefresh(jeuFen);

    jeuCarte = createWindowJeu(jeuFen,1,1);
    wbkgdset(jeuCarte,COLOR_PAIR(2));
    remplireFenCarte(jeuCarte,shmid,&vPosY,&vPosX);
    /*printf("\nposy=%d&posx=%d\n",vPosY,vPosX);*/
    wrefresh(jeuCarte);


    wbkgd(infosText, COLOR_PAIR(2));
    
    box(infosFen,0,0);
    
    wrefresh(infosText);
    wrefresh(infosFen);
    
    move(LINES-1,0);
    printw("Utilisez les fleches directionnelles ; pressez F2 pour quitter...");
    leSegment = getSegmentVals(shmid);
    printf("%ls",leSegment->largeur);
    
    /* Routine principale */
    if(!hasLost(shmid) && !hasWon(shmid)) {
        while((ch = getch()) != KEY_F(2)) {
            if(ch==ERR) {
                /* on a pas bouger */
                if(segmentEquals(leSegment,getSegmentVals(shmid))) {
                    /*les deux semgents sont différents*/
                    printf("pas pareil");

                    sleep(1);
                }
            }
            /*********************************Pb au niveau du segmentequals -> core dumped même avec la suite isolée*/
            vPosXTemp=vPosX;
            vPosYTemp=vPosY;
            msgDirect="";
            /* On efface le curseur */
            wattron(jeuCarte,COLOR_PAIR(14));
            mvwaddch(jeuCarte,vPosY, vPosX, ' ');
            wattron(jeuCarte,COLOR_PAIR(14));

           
            /* On calcule la nouvelle position */
            switch(ch) 
            {
              case KEY_LEFT:
                    if(vPosX > 0) vPosX--;
                    msgDirect = "a gauche";
                break;
              case KEY_RIGHT:
                    if(vPosX < LARGEURC - 1) vPosX++;
                    msgDirect = "a droite";
                break;
              case KEY_UP:
                    if(vPosY > 0) vPosY--;
                    msgDirect = "en haut";
                break;
              case KEY_DOWN:
                    if(vPosY < HAUTEURC - 1) vPosY++;
                    msgDirect = "en bas";
                break;
            }
        }
    }
           /* okMove=bougerValGuerrier(shmid, vPosY, vPosX);
            if(okMove && okMove!=4)
                {
                msgInfo="mur deja découvert";
                wrefresh(jeuCarte);
                if(okMove==1)
                    {
                    wattron(jeuCarte,COLOR_PAIR(13));
                    mvwaddch(jeuCarte,vPosY, vPosX, ' ');
                    wattron(jeuCarte,COLOR_PAIR(13));
                    msgInfo="mur invisible";
                    wmove(buttonFen,2,1);
                    wclrtoeol(buttonFen);
                    wprintw(buttonFen,"Mur : %d/%d",getNbVal(shmid, 3), getNbVal(shmid, 1)+getNbVal(shmid, 3));
                    wrefresh(buttonFen);
                    }
                else if(okMove==2) msgInfo="mur visible";
                vPosX=vPosXTemp;
                vPosY=vPosYTemp;
                wattron(jeuCarte,COLOR_PAIR(9));
                mvwaddch(jeuCarte,vPosYTemp, vPosXTemp, ' ');
                wattron(jeuCarte,COLOR_PAIR(9));

                wprintw(infosText,"Vous etes aller %s : %s (%d,%d)\n",msgDirect,msgInfo,vPosX,vPosY);
                wmove(buttonFen,0,1);
                wclrtoeol(buttonFen);*/ /* on supprime la ligne */
                /*wprintw(buttonFen,"Vies : %c",getNbVie(shmid));
                wrefresh(buttonFen);
            }

            else
                {*/
                /* On affiche le curseur */
                /*if(okMove==0) setValCase(shmid,vPosYTemp,vPosXTemp,'4');
                wattron(jeuCarte,COLOR_PAIR(9));
                mvwaddch(jeuCarte,vPosY, vPosX, ' ');
                wattron(jeuCarte,COLOR_PAIR(9));
                wprintw(infosText,"Vous etes aller %s : pas de mur (%d,%d)\n",msgDirect,vPosX,vPosY);
                
                wmove(buttonFen,1,1);
                wclrtoeol(buttonFen);
                wprintw(buttonFen,"Deplacement : %d",getNbVal(shmid, 4));
                wrefresh(buttonFen);
                }
            wrefresh(jeuCarte);
            wrefresh(infosText);
            wrefresh(infosFen);
            if((getNbVie(shmid)=='0') || (vPosY==8 && vPosX==29))break;
        }
    }*/

    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t*********11     ");
    if(hasLost(shmid)) {
        move(LINES-1,0); /* on va à la dernière ligne du terminal */
        clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
        printw("Vous avez perdu ! Appuyez sur F2 pour quitter.");
        while((ch = getch()) != KEY_F(2)) {}
    }
    else if(hasWon(shmid)){
        move(LINES-1,0); /* on va à la dernière ligne du terminal */
        clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
        printw("Vous avez gagné ! Appuyez sur F2 pour quitter.");
        while((ch = getch()) != KEY_F(2)) {}
    }
    printf("*********12     ");

    /*while(!stop) pause();*/
    /* Suppression de la fenêtre */


    /* interdiction au minautore de se connecter */
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        if(errno==EAGAIN)
            perror("Le joueur n'est pas connecté "); 
        else
            perror("Erreur lors de l'operation sur le semaphore ");
        exit(EXIT_FAILURE);
    }

    /*réalisation de V(S1)*/
    op.sem_num = 1;
    op.sem_op = 1;
    op.sem_flg = 0;
    if(semop(semid, &op, 1) == -1) {
        perror("Erreur lors de l'operation sur le semaphore ");
        exit(EXIT_FAILURE);
    }
    printf("*********13     ");

    delwin(jeuCarte);
    delwin(jeuFen);

    delwin(infosText);
    delwin(infosFen);

    delwin(buttonFen);

    /* Arrêt de ncurses */
    ncurses_stopper();
    
    printf("Envoie de la requête de deco.\n");

    requete.type = TYPE_REQ_DECO;
    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        perror("Erreur lors de l'envoi de la requete ");
        exit(EXIT_FAILURE);
    }

    
    printf("Requete envoyee.\n");
	return EXIT_SUCCESS;
    }
