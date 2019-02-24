#define LARGEUR 75      /* Largeur de la fenêtre */
#define HAUTEUR 27      /* Hauteur de la fenêtre */
#define POSX    20      /* Position horizontale de la fenêtre */
#define POSY    5       /* Position verticale de la fenêtre */

#include "structures.h"
#include "carte.h"
#include <errno.h>
#include <signal.h>    /* Pour sigXXX */
#include <sys/types.h> /* Pour pid_t */

int stop = 0;

void handler(int signum, siginfo_t* info, void* rien){
    if(info->si_pid==0) stop=1;
    else stop=2;
    /*exit(EXIT_SUCCESS);*/
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
    
    WINDOW *jeuCarte,*jeuFen, *infosFen,*infosText, *buttonFen;
    int infosFenY = 10; /* taille en Y de la fenetre */  

    int msqid,shmid,semid;
    requete_t requete;
    reponse_t reponse;
    int ch,posX,okMove=0;
    int vPosX=0,vPosY=8;
    int vPosXTemp,vPosYTemp;
    char * msgDirect="";
    char * msgInfo="";
    struct sembuf op;
    struct sigaction action;
    segment_t *result;
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

    if((msqid = msgget((key_t)atoi(argv[1]), 0)) == -1) {
        ncurses_stopper();
        perror("Erreur lors de la recuperation de la file ");
        fprintf(stderr,"Vérifier la connexion du controleur.\n");
        exit(EXIT_FAILURE);
    }

    requete.type = TYPE_REQ_SMPTS;
    requete.typeActeur = TYPE_JOUEUR;
    requete.pid = getpid();


    if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
        ncurses_stopper();
        perror("Erreur lors de l'envoi de la requete ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }
    
    if(msgrcv(msqid, &reponse, sizeof(reponse_t) - sizeof(long), TYPE_REP_SMPTS, 0) == -1) {
        ncurses_stopper();
        perror("Erreur lors de la reception d'une requete ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /* Recuperation du segment de memoire partagee */
    if((shmid = shmget((key_t)reponse.rep.r1.cleSMP, 0, 0)) == -1) {
        ncurses_stopper();
        perror("Erreur lors de la recuperation du segment de memoire partagee ");
        fprintf(stderr,"Vérifier la connexion du controleur.\n");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /* Recuperation du tableau de semaphores */
    if((semid = semget((key_t)reponse.rep.r1.cleTS, 0, 0)) == -1) {
        ncurses_stopper();
        perror("Erreur lors de la recuperation du tableau de semaphores ");
        fprintf(stderr,"Vérifier la connexion du controleur.\n");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /* donner l'information de l'autorisation de connexion pour les minautores*/
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1) {
        ncurses_stopper();
        if(errno==EAGAIN)
            perror("Impossible de donner l'autorisation ");
        else
            perror("Erreur lors de l'operation sur le semaphore ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /*réalisation de P(S1,1)*/
    op.sem_num = 1;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;
    if(semop(semid, &op, 1) == -1){
        ncurses_stopper();
        if(errno == EAGAIN)
            perror("Nombre maximal de joueur atteint ");
        else
            perror("Erreur lors de l'opération sur le semaphore.");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    /*mise en place du gestionanire pour SIGINT*/
    action.sa_sigaction=handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &action, NULL) == -1){
        ncurses_stopper();
        perror("Erreur de positionnement ");
        sendDeco(requete,msqid);
        exit(EXIT_FAILURE);
    }

    result = getSegmentVals(shmid);

    /*Calcul de la position centrale*/
    posX = COLS / 2 - 1;
    move(0,0);
    
    infosFen = newwin(infosFenY, posX, 0, 0);
    mvwaddch(infosFen, 0, 0, ' ');

    infosText = derwin(infosFen,8, posX-2,1,1);
    scrollok(infosText, TRUE);
    


    buttonFen = newwin(8,18,11,LARGEURC+3);
    wmove(buttonFen,0,1);
    wprintw(buttonFen,"Vies : %c",*result->nbVie);
    wbkgd(buttonFen,COLOR_PAIR(15));

    /* affichage de la légende */

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
    remplireFenCarte(jeuCarte,shmid,&vPosY,&vPosX);
    wbkgdset(jeuCarte,COLOR_PAIR(2));
    wrefresh(jeuCarte);


    wbkgd(infosText, COLOR_PAIR(2));
    
    box(infosFen,0,0);
    
    wrefresh(infosText);
    wrefresh(infosFen);
    
    move(LINES-1,0);
    printw("Utilisez les fleches directionnelles ; pressez F2 pour quitter...");
    
    /* Routine principale */
    if(!hasLost(result) && !hasWon(result)) {
        while((ch = getch()) != KEY_F(2) && !stop) {
            if(ch==ERR || (ch != KEY_LEFT && ch != KEY_RIGHT && ch != KEY_UP && ch != KEY_DOWN)) {
                /* on a pas bouger */
                jeuCarte = createWindowJeu(jeuFen,1,1);
                remplireFenCarte(jeuCarte,shmid,&vPosY,&vPosX);
                wrefresh(jeuCarte);
            }
            else {
                /*********************************Pb au niveau du segmentequals -> core dumped même avec la suite isolée*/
                jeuCarte = createWindowJeu(jeuFen,1,1);
                wrefresh(jeuCarte);
                vPosXTemp=vPosX;
                vPosYTemp=vPosY;
                msgDirect="";

                /* On efface le curseur 
                wattron(jeuCarte,COLOR_PAIR(14));
                mvwaddch(jeuCarte,vPosY, vPosX, ' ');
                wattron(jeuCarte,COLOR_PAIR(14));*/

            
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
                okMove=bougerValGuerrier(shmid, vPosY, vPosX);
                if(okMove && okMove!=4)
                    {
                    msgInfo="mur deja découvert";
                    if(okMove==1)
                        {
                        setValCase(shmid,vPosY,vPosX,'3');
                        msgInfo="mur invisible";
                        wmove(buttonFen,2,1);
                        wclrtoeol(buttonFen);
                        wprintw(buttonFen,"Mur : %d/%d",getNbVal(shmid, 3), getNbVal(shmid, 1)+getNbVal(shmid, 3));
                        wrefresh(buttonFen);
                        }
                    else if(okMove==2) msgInfo="mur visible";
                    vPosX=vPosXTemp;
                    vPosY=vPosYTemp;
                    
                    setValCase(shmid,vPosYTemp,vPosXTemp,ZERO_ASCII+10);

                    wprintw(infosText,"Vous etes aller %s : %s (%d,%d)\n",msgDirect,msgInfo,vPosX,vPosY);
                }

                else
                    {
                    /* On affiche le curseur */
                    if(okMove==0) setValCase(shmid,vPosYTemp,vPosXTemp,'4');
                    if(okMove==4) {
                        setValCase(shmid,vPosY,vPosX,ZERO_ASCII+10);
                        setValCase(shmid,vPosYTemp,vPosXTemp,'4');
                    }
                    wprintw(infosText,"Vous etes aller %s : pas de mur (%d,%d)\n",msgDirect,vPosX,vPosY);
                    }
                    
                remplireFenCarte(jeuCarte,shmid,&vPosY,&vPosX);
                wrefresh(jeuCarte);
                wrefresh(infosText);
                wrefresh(infosFen);
                if(hasLost(result) || hasWon(result))break;
            }
        wmove(buttonFen,0,1);
        wclrtoeol(buttonFen);/* on supprime la ligne */
        wprintw(buttonFen,"Vies : %c",*result->nbVie);
        wrefresh(buttonFen);
        }
    }
    if(hasLost(result) || hasWon(result)) {
        requete.type = TYPE_REQ_EVT;
        if(msgsnd(msqid, &requete, sizeof(requete_t) - sizeof(long), 0) == -1) {
            ncurses_stopper();
            perror("Erreur lors de l'envoi de la requete ");
            sendDeco(requete,msqid);
            exit(EXIT_FAILURE);
        }
        
        if(msgrcv(msqid, &reponse, sizeof(reponse_t) - sizeof(long), TYPE_REP_EVT, 0) == -1) {
            ncurses_stopper();
            perror("Erreur lors de la reception d'une requete ");
            sendDeco(requete,msqid);
            exit(EXIT_FAILURE);
        }
        if(hasLost(result)) {
            move(LINES-1,0); /* on va à la dernière ligne du terminal */
            clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
            printw("Vous avez perdu face à %d minotaures ! Appuyez sur F2 pour quitter.",reponse.rep.r2.nbMino);
            while((ch = getch()) != KEY_F(2)) {}
        }
        else if(hasWon(result)){
            move(LINES-1,0); /* on va à la dernière ligne du terminal */
            clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
            printw("Vous avez gagné face à %d minotaures ! Appuyez sur F2 pour quitter.",reponse.rep.r2.nbMino);
            while((ch = getch()) != KEY_F(2)) {}
        }
    }
    /*while(!stop) pause();*/
    /* Suppression de la fenêtre */
    delwin(jeuCarte);
    delwin(jeuFen);

    delwin(infosText);
    delwin(infosFen);

    delwin(buttonFen);

    /* Arrêt de ncurses */
    ncurses_stopper();
    

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

    if(stop<2) {
        printf("Envoie de la requête de deco.\n");
        sendDeco(requete,msqid);
        printf("Requete envoyee.\n");
        while(!stop)pause();
    }
    else if(stop==2) {
        printf("Signal recu du contrôleur, je me déconnecte !\n");
    }

	return EXIT_SUCCESS;
    }
