
#include <stdio.h>     /* Pour printf, scanf */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */

#include "carte.h"
#include "ncurses.h"

#define LARGEUR 75      /* Largeur de la fenêtre */
#define HAUTEUR 27      /* Hauteur de la fenêtre */
#define POSX    20      /* Position horizontale de la fenêtre */
#define POSY    5       /* Position verticale de la fenêtre */

int main(int argc, char* argv[])
	{
    int ch,posX,okMove=0;
    int vPosX=0,vPosY=8;
    int vPosXTemp,vPosYTemp;
	int fdCarte;
    char * msgDirect;
    char * msgInfo;

    int infosFenY = 10; /* taille en Y de la fenetre */

	WINDOW *jeuCarte,*jeuFen, *infosFen,*infosText, *buttonFen;

    if(argc!=2) {
        fprintf(stderr, "Usage: %s nomCarte\n", argv[0]);
        fprintf(stderr, "Ou:\n");
        fprintf(stderr, "\tnomCarte : nom de la carte ou de la sauvegarde sur lequel jouer\n");
        fprintf(stderr, "\tformat nomCarte : XXX_YYY ou XXX est le nom de la carte et YYY numéro de version.\n");
        exit(EXIT_FAILURE);
    }

    if((fdCarte = editCarte(argv[1])) == -1) {
        fprintf(stderr, "Le carte '%s' n'existe pas\n", argv[1]);
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



    /*Calcul de la position centrale*/
    posX = COLS / 2 - 1;
    move(0,0);
    
    infosFen = newwin(infosFenY, posX, 0, 0);
    mvwaddch(infosFen, 0, 0, ' ');

    infosText = derwin(infosFen,8, posX-2,1,1);
    scrollok(infosText, TRUE);
    


    buttonFen = newwin(8,18,11,LARGEURC+3);
    wmove(buttonFen,0,1);
    wprintw(buttonFen,"Vies : %c/%c",getNbVieRestante(fdCarte),getNbVieDepart(fdCarte));
    wbkgd(buttonFen,COLOR_PAIR(15));

    /* affichage de la légende */
    wmove(buttonFen,1,1);
    wprintw(buttonFen,"Deplacement : %d",getNbVal(fdCarte, 4));

    wmove(buttonFen,2,1);
    wprintw(buttonFen,"Mur : %d/%d",getNbVal(fdCarte, 3), getNbVal(fdCarte, 1)+getNbVal(fdCarte, 3));


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

	jeuCarte = createWindowJeu(jeuFen,fdCarte,1,1); /** le 2 ajouté correspond au décalage pour les bordures de la fenètre mère */
    wbkgdset(jeuCarte,COLOR_PAIR(2));
    remplireFenCarte(jeuCarte,fdCarte,&vPosY,&vPosX);
    wrefresh(jeuCarte);


    wbkgd(infosText, COLOR_PAIR(2));
    
    box(infosFen,0,0);
    
    wrefresh(infosText);
    wrefresh(infosFen);
    
    move(LINES-1,0);
    printw("Utilisez les fleches directionnelles ; pressez F2 pour quitter...");
    /* Routine principale */
    if(!hasLost(fdCarte) && !hasWon(fdCarte)) {
        while((ch = getch()) != KEY_F(2)) {
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
            okMove=bougerValGuerrier(fdCarte, vPosY, vPosX);
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
                    wprintw(buttonFen,"Mur : %d/%d",getNbVal(fdCarte, 3), getNbVal(fdCarte, 1)+getNbVal(fdCarte, 3));
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
                wclrtoeol(buttonFen); /* on supprime la ligne */
                wprintw(buttonFen,"Vies : %c/%c",getNbVieRestante(fdCarte),getNbVieDepart(fdCarte));
                wrefresh(buttonFen);
                }
            else
                {
                /* On affiche le curseur */
                wattron(jeuCarte,COLOR_PAIR(9));
                mvwaddch(jeuCarte,vPosY, vPosX, ' ');
                wattron(jeuCarte,COLOR_PAIR(9));
                wprintw(infosText,"Vous etes aller %s : pas de mur (%d,%d)\n",msgDirect,vPosX,vPosY);
                
                wmove(buttonFen,1,1);
                wclrtoeol(buttonFen);
                wprintw(buttonFen,"Deplacement : %d",getNbVal(fdCarte, 4));
                wrefresh(buttonFen);
                }
            wrefresh(jeuCarte);
            wrefresh(infosText);
            wrefresh(infosFen);
            if((getNbVieRestante(fdCarte)=='0') || (vPosY==8 && vPosX==29))break;
        }
    }
    if(hasLost(fdCarte)) {
        move(LINES-1,0); /* on va à la dernière ligne du terminal */
        clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
        printw("Vous avez perdu ! Appuyez sur F2 pour quitter.");
    }
    else if(hasWon(fdCarte)){
        move(LINES-1,0); /* on va à la dernière ligne du terminal */
        clrtoeol(); /* on supprime la ligne pour afficher le message d'avoir perdu */
        printw("Vous avez gagné ! Appuyez sur F2 pour quitter.");
    }
    while((ch = getch()) != KEY_F(2)) {}
    /* Suppression de la fenêtre */
        
    delwin(jeuCarte);
    delwin(jeuFen);

    delwin(infosText);
    delwin(infosFen);

    delwin(buttonFen);

    /* Fermeture du fichier */
    closeCarte(fdCarte);

    /* Arrêt de ncurses */
    ncurses_stopper();
    
	return EXIT_SUCCESS;
	}
